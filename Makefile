# DPKG-Hider Makefile

# Compilation Parameters
CC=gcc
CFLAGS= -Iincludes -Wextra -Wall
SOURCES= dpkg_hider.c

all:
	$(CC) $(SOURCES) $(CFLAGS) -o dpkg-hider
clean:
	rm -f dpkg-hider
