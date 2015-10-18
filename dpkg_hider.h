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

#ifndef __DPKG_HIDER_H__
#define __DPKG_HIDER_H__

/*******************************************************************
* Constants & Macros
*******************************************************************/

typedef enum {
	false = 0,
	true,
} bool;

enum errors {
	ERROR = -1,
	SUCCESS = 0,
	ERR_PACKAGE_NOT_FOUND = 1000,
	ERR_OPEN_STATUS_FILE_READ,
	ERR_SEEK_STATUS_FILE_END,
	ERR_INVALID_STATUS_FILE_SIZE,
	ERR_SEEK_STATUS_FILE_START,
	ERR_MEM_ALLOC,
	ERR_READ_FROM_STATUS_FILE,
	ERR_MEM_ALLOC_NEW_CONT,
	ERR_OPEN_STATUS_FILE_WRITE,
	ERR_STATUS_FILE_WRITE,
	ERR_STATUS_FILE_ACCESS,
	
};

#define OUT

#define FILE_START (0)
#define VALID_ARGS_COUNT (2)
#define MAX_PACKAGE_LINE_SIZE (1024)

#define DPKG_STATUS_PATH ("/var/lib/dpkg/status")
#define PACKAGE_STR ("Package: ")

#endif /* __DPKG_HIDER_H__*/
