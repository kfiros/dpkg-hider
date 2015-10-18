/*******************************************************************
* Project:	DPKG-Hider
*
* Author:	Kfiros (Kfir Shtober)
* Year:		2015	
*
* File:		dpkg_hider.c
* Description:	DPKG-Hider is a tiny utility which hides installed
*		linux packages from the `dpkg` tool. It's essential
*		to understand that once a package is hidden, `dpkg`
*		wouldn't recognize it's installed, therefore,
*		wouldn't rely on it.
*******************************************************************/

/*******************************************************************
* Includes
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/stat.h>
#include "dpkg_hider.h"

/*******************************************************************
* Name: 	check_preconditions
* Description:	Checks preconditions for running the DPKG-Hider.
*		Tests whether the `status` file exists, and we hold 
*		the required permissions
*******************************************************************/
static int check_preconditions() {
	int ret = SUCCESS;
	int call_rv; 

	/* Check for status file existence and permissions */
	call_rv = access(DPKG_STATUS_PATH, F_OK | R_OK | W_OK);
	if (-1 == call_rv) {
		ret = ERR_STATUS_FILE_ACCESS;
		goto cleanup;		
	}

cleanup:
	return ret;
}

/*******************************************************************
* Name: 	read_status_file
* Description:	Reads the dpkg status file, and saves it in a new
*		allocated memory.
*******************************************************************/
static int read_status_file(OUT char ** src_cont, OUT size_t * src_size) {
	int ret = SUCCESS;
	int call_rv;
	FILE * status_fd = NULL;
	size_t file_size = 0;

	status_fd = fopen(DPKG_STATUS_PATH, "rb");
	if (NULL == status_fd) {
		ret = ERR_OPEN_STATUS_FILE_READ;
		goto cleanup;
	}

	call_rv = fseek(status_fd, 0L, SEEK_END);
	if (-1 == call_rv) {
		ret = ERR_SEEK_STATUS_FILE_END;
		goto cleanup;
	}

	/* Obtain file size */	
	file_size = ftell(status_fd);
	if (file_size <= 0) {
		ret = ERR_INVALID_STATUS_FILE_SIZE;	
		goto cleanup;	
	}

	/* Seek to the beginning of the file */
	call_rv = fseek(status_fd, 0L, SEEK_SET);
	if (-1 == call_rv) {
		ret = ERR_SEEK_STATUS_FILE_START;
		goto cleanup;
	}	

	/* Allocate memory for the file's content */
	*src_cont = calloc(1, file_size);
	if (NULL == *src_cont) {
		ret = ERR_MEM_ALLOC;
		goto cleanup;
	}

	/* Read the status file content */
	call_rv = fread(*src_cont, file_size, 1, status_fd);
	if (1 != call_rv) {
		ret = ERR_READ_FROM_STATUS_FILE;
		goto cleanup;
	}

	/* Change the input src_size variable */
	*src_size = file_size;
	
cleanup:
	if (NULL != status_fd) {
		fclose(status_fd);
	}

	return ret;
}


/*******************************************************************
* Name: 	write_status_file
* Description:	Writes the new changed content to the status file.
*******************************************************************/
static int write_status_file(char * dst_cont) {
	int ret = SUCCESS;
	int call_rv;
	FILE * status_fd = NULL;
	size_t dst_cont_len = 0;

	status_fd = fopen(DPKG_STATUS_PATH, "wb");
	if (NULL == status_fd) {
		ret = ERR_OPEN_STATUS_FILE_WRITE;
		goto cleanup;
	}

	dst_cont_len = strlen(dst_cont);
	if (0 == dst_cont_len) {
		goto cleanup;
	}

	call_rv = fwrite(dst_cont, dst_cont_len, 1, status_fd);
	if (1 != call_rv) {
		ret = ERR_STATUS_FILE_WRITE;
		goto cleanup;
	}
	
cleanup:
	if (NULL != status_fd) {
		fclose(status_fd);
	}

	return ret;
}

/*******************************************************************
* Name: 	gen_status_without_pkg
* Description:	Changes the status file contents, in a way the given
*		package would be hidden. It stores the new content
*		in dst_cont.
*******************************************************************/
static int gen_status_without_pkg(char * src_cont,
			OUT char * dst_cont,
			const char * pkg_name) {
	int ret = SUCCESS;
	char * src_cont_ptr = src_cont, * dst_cont_ptr = dst_cont;
	char * found_pkg = NULL, * next_pkg = NULL;
	char pkg_line[MAX_PACKAGE_LINE_SIZE] = {0};
	
	/* Format the package line we are searching for */
	snprintf(pkg_line, MAX_PACKAGE_LINE_SIZE, "%s%s\n", PACKAGE_STR, pkg_name);

	/* Search for the instance of the package line */
	found_pkg = strstr(src_cont_ptr, pkg_line);
	if (NULL == found_pkg) {
		ret = ERR_PACKAGE_NOT_FOUND;
		goto cleanup;
	}

	/* Locate any next package instance */
	next_pkg = strstr(found_pkg + 1 , PACKAGE_STR);

	if (FILE_START == found_pkg) {
		if (NULL == next_pkg) {
			/* Our package is the only one */
			dst_cont[0] = '\0';
			goto cleanup;
		} else {
			/* Our package is the first one. Therefore,
				we just copy the latter content. */
			strncpy(dst_cont_ptr, next_pkg,
				strlen(src_cont_ptr) - (next_pkg - src_cont_ptr));
		}
	}

	else if (NULL != next_pkg) {
		/* Our package is in the middle, and not the last one */
		strncpy(dst_cont_ptr, src_cont_ptr, found_pkg - src_cont_ptr);
		dst_cont_ptr += (found_pkg - src_cont_ptr);

		strncpy(dst_cont_ptr, next_pkg,
			strlen(next_pkg));
	}
	else {
		/* Our package is the last package */
		strncpy(dst_cont_ptr, src_cont_ptr,
				found_pkg - src_cont_ptr);
	}

cleanup:
	return ret;
}

/*******************************************************************
* Name: 	hide_pkg
* Description:	This function responsibles for calling the needed
*		sub functions, in order to hide the given package.
*******************************************************************/
static int hide_pkg(char * pkg_to_hide) {
	int ret = SUCCESS;
	int call_rv;
	char * src_status_cont = NULL, * dst_status_cont = NULL;
	size_t src_status_size = 0;

	call_rv = read_status_file(&src_status_cont, &src_status_size);
	if (SUCCESS != call_rv) {
		ret = call_rv;
		goto cleanup;
	}
	
	/* The new file size shouldn't be much less than the original */
	dst_status_cont = calloc(1, src_status_size);
	if (NULL == dst_status_cont) {
		ret = ERR_MEM_ALLOC_NEW_CONT;
		goto cleanup;
	}

	call_rv = gen_status_without_pkg(src_status_cont, dst_status_cont, pkg_to_hide);	
	if (SUCCESS != call_rv) {
		if (ERR_PACKAGE_NOT_FOUND == call_rv) {
			fprintf(stderr, "[*] Package does not exist\n");
		}
		ret = call_rv;
		goto cleanup;
	} 

	call_rv = write_status_file(dst_status_cont);
	if (SUCCESS != call_rv) {
		ret = call_rv;
		goto cleanup;
	}

cleanup:
	/* Free resources if needed */
	if (NULL != src_status_cont) {
		free(src_status_cont);
	}
	if (NULL != dst_status_cont) {
		free(dst_status_cont);
	}
	

	return ret;

}

/*******************************************************************
* Name: 	main
* Description:	Main function of the program
*******************************************************************/
int main(int argc, char * argv[]) {
	int ret = SUCCESS;
	int call_rv;
	char * pkg_to_hide = NULL;	

	fprintf(stdout, "[*] dpkg-hider (Kfiros 2015) \n");

	if (VALID_ARGS_COUNT != argc) {
		fprintf(stderr, "[*] USAGE: dpkg-hider package-name \n");
		exit(ERROR);
	}

	pkg_to_hide = argv[1];	

	call_rv = check_preconditions();
	if (SUCCESS != call_rv) {
		fprintf(stderr, "[*] Error: Preconditions are not set. Exiting... \n");
		ret = call_rv;
		goto cleanup;
	}	

	fprintf(stdout, "[*] Hiding the following package: %s \n", pkg_to_hide);
	call_rv = hide_pkg(pkg_to_hide);
	if (SUCCESS != call_rv) {
		fprintf(stderr, "[*] Error has been encountered. Exiting...\n");
		ret = call_rv;
		goto cleanup;
	}

	/* If everything went well... */
	fprintf(stdout, "[*] The package is now hidden. \n");

cleanup:
	return ret;
}

