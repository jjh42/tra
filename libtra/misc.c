/***************************************************************************
                          misc.c  -  Implements a few standalone functions
                             -------------------
    begin                : Wed Dec 24 2003
    copyright            : (C) 2003 by jjhunt
    email                : jjhunt@r2d2
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>

#include <config.h>

#include "libtra_internal.h"

#ifndef      get_current_dir_name
char *get_current_dir_name(void);
#endif

/* Just like malloc only on failure to allocate memory it outputs an error message */
void *_safe_alloc(size_t size, int line, const char *file)
{
    void *ptr = malloc(size);
    if(!ptr)
        _report_error("Unable to allocate memory", line, file);

    return ptr;
}

void _report_error(const char *err, int line, const char *file)
{
    fprintf(stderr, "libtra:%s:%d: '%s' libtra version %s\n",  file, line, err, VERSION);
}

int move_file(const char *oldname, const char *newname)
{
    int retval;
    char *buf;
    buf = safe_alloc(strlen(oldname) + strlen(newname) + strlen("mv   "));
    if(!buf)
        return -1;
    /* Use the UNIX command mv to do the work for us */
    strcpy(buf, "mv ");
    strcat(buf, oldname);
    strcat(buf, " ");
    strcat(buf, newname);

    retval = system(buf);
    free(buf);

    if(retval != 0) {
        report_error("Move command failed");
    }
    return retval;    
}

int set_readonly(const char *newname)
{
    mode_t mode;

    /* Symlinks don't have modes (and it affects instead the files they point to so just ignore the command if this is a symlink. */
    if(issymlink(newname))
            return 0;
        
    mode = getmode(newname);
    
    /* Remove all the writable characteristics of this file */
    mode = (mode & (~(S_IWGRP |  S_IWOTH |  S_IWUSR)));

    /* Change the mode  */
    chmod(newname, mode);

    return 0;
}

mode_t getmode(const char *filename)
{
    struct stat buf;

    if(lstat(filename, &buf) != 0) {
        report_error("Erroring stating file");
        return 0;
    }

    return buf.st_mode;
}

int issymlink(const char *filename)
{
    struct stat buf;

    if(lstat(filename, &buf) != 0) {
        report_error("Erroring stating file");
        return FALSE;
    }

    return S_ISLNK(buf.st_mode);    
}

int gettitleandpath(const char *filename, char **filepath, char **filetitle)
{
	char *cwd;
	char *absname;
	int freeabsname;
	char *firstslash;
	
	freeabsname = 0;
	/* First get the the absolute file title. */
	if(filename[0] != '/') {
		/* Not an absolute filename */
		cwd = get_current_dir_name();
		if(!cwd) {
			report_error("Unable to get cwd");
			return -1;
		}
		/* Put the cwd at the beginning  */
		absname = safe_alloc(strlen(cwd) + strlen(filename) + 1);
		strcpy((char*)absname, cwd);
		strcat((char*)absname, "/");
		strcat((char*)absname, filename);
		freeabsname = 1;
		free(cwd);
	}
      else
      	absname = (char*)filename;

       	
	/* Have the absolute name now extract the filetitle */
	/* Find the first slash and then copy everything after this into the file title. */
	firstslash = rindex(absname, '/');
	if(!firstslash) {
		report_error("Unable to find / in the filename");
		if(freeabsname)
			free((void*)absname);
		return -1;
	}
      (*filetitle) = safe_alloc(strlen(firstslash + 1));
      if(!(*filetitle)) {
  	        if(freeabsname)
			  free((void*)absname);
		  return -1;
	}
	strcpy((char*)(*filetitle), (firstslash + 1));


	/* Now copy in the path */
	(*filepath) = safe_alloc((firstslash - absname) + 1);
	if(!(*filepath)) {
		free((void*)absname);
		free((void*)(*filetitle));
	}
	strncpy((char*)*filepath, absname, (firstslash - absname)); /* Will not be null terminated */
      (*filepath)[firstslash - absname] = 0;

      return 0;
}


char *getworkingfilename(char *dbname)
{
    char *firstslash;
    char *secondslash;

    firstslash = rindex(dbname, '/');
    firstslash[0] = 0; /* Put a NULL to we can find the secondslash. */
    secondslash = rindex(dbname, '/');

    /* Need to remove everything between the first and second slash. */
    firstslash++;
    secondslash++;
    while(firstslash[0]) {
        secondslash[0] = firstslash[0];
        secondslash++;
        firstslash++;
    }
    /* Append a NULL */
    secondslash[0] = 0;

    return dbname;
}
