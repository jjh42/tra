/***************************************************************************
                          libtra.c  -  description
                             -------------------
    begin                : Wed Dec 24 2003
    copyright            : (C) 2003 by Jonathan Hunt
    email                : jhuntnz@users.sourceforge.net
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
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <unistd.h>
 
#include "libtra.h"
#include "libtra_internal.h"

struct trash_bin_struct;
typedef struct trash_bin_struct trash_bin_t;


/* Initialize the trashbin_t for use. Note: Changes made here may also need to be made to trash_free that is
 * called to free the trash pointer.
 */
trashbin_t *trash_init(const char *dir)
{
    trashbin_t *bin;
    const char *envval;
    int err;
    char *temp;

    bin  = (trashbin_t*)safe_alloc(sizeof(trashbin_t));
    if(!bin)
        return NULL;

    /* Set the working directory of the rubbish bin */
    if(dir) {   /* If specified use that */
        bin->path = strdup(dir);
    }

    /* First check for environment variable named TRASH otherwise take HOME/.trash */
    envval = getenv("TRASH");
    if(envval) {   
        bin->path = strdup(envval);
    }
    else {
        envval = getenv("HOME");
        if(envval) {
                /* Append "/.trash" onto the end */
                char *path = safe_alloc(strlen(envval) + strlen(TRASH_FOLDER_EXT));
                strcpy(path, envval);
                strcat(path, TRASH_FOLDER_EXT);
                bin->path = path;                
        }        
    }    

    if(!bin->path) {
        report_error("Unable to get a directory");
        free(bin);
        return NULL;
    }

    /* Create the trash directory and the subdir files if it does not exist */
    err = mkdir(bin->path, CREATE_DIR_MODE);
    /* Make the files subdir */
    temp = safe_alloc(strlen(bin->path) + strlen(DB_FOLDER_EXT));
    strcpy(temp, bin->path);
    strcat(temp, DB_FOLDER_EXT);
    err = mkdir(temp, CREATE_DIR_MODE);
    bin->dbpath = temp;
    
    
    return bin;
}

void trash_free(trashbin_t *bin)
{
    assert_trashvalid(bin);
    free((void*)bin->path);
    free((void*)bin->dbpath);
    free(bin);
}

void assert_trashvalid(trashbin_t*bin)
{
    assert(bin);
    assert(bin->path);
    assert(bin->dbpath);
}

int trash_file(trashbin_t *bin, const char *filename)
{
    
    char *newname;
    int retval;
    trashitem_t item;
    char *dbname;
    int i;

    i = 0;
    
    assert_trashvalid(bin);

    /* First get relevant info about the item. */
    item.datedeletion = time(NULL);
    /* Get the old mode before it is set readonly */
    item.filemode = getmode(filename);
    
    if(gettitleandpath(filename, &item.filepath, &item.filetitle) != 0) {
        report_error("Unable to get tile and pathname from file");
	  return -1;
    }    
    /* Get a unique filename for the data file */
    dbname = safe_alloc(strlen(bin->dbpath) + strlen("/XXXXXX"));
    if(!dbname)  {
	  free((void*)item.filepath);
	  free((void*)item.filetitle);
        return -1;
    }
    strcpy(dbname, bin->dbpath);
    strcat(dbname, "/XXXXXX");
    retval = mkstemp(dbname);
    if(retval < 0) {
        report_error("Unable to get a unique filename");
        return -1;
    }

    /* First create an entry in the db */
    if(createdb_entry(dbname, &item) != 0) {
        free((void*)item.filepath);
        free((void*)item.filetitle);
        free((void*)dbname);
        return -1;
    }
    free((void*)item.filepath);
    free((void*)item.filetitle);

       /* Now get a unique name for the file as well by grabbuing the last six digits of the filename */
    newname = safe_alloc(strlen(bin->path) + strlen("/XXXXXX"));
    if(! newname) {
        free((void*)dbname);
        return -1;
    }
    strcpy(newname, bin->path);
    strcat(newname, dbname + strlen(dbname) - 7);    

    /* Now actually move the file to the destination and set to read only */
    if(move_file(filename, newname) != 0) {
	  /* Remove the db entry */
	  unlink(dbname);
	  free((void*)newname);
	  free((void*)dbname);
	  return -1;	         
    }
    
    if(set_readonly(newname) != 0) {
	  free(newname);
	  free(dbname);
	  return -1;
    }

    free(newname);
    free(dbname);    
    
    return 0;

}

int trash_restorefilename(trashbin_t *bin, const char *name)
{
    char *filetitle;
    char *filepath;
    trashiterator_t *it;
    trashitem_t *cur;
    trashitem_t *bestmatch;
    int ret;

    if(gettitleandpath(name, &filepath, &filetitle) != 0)
        return -1;

    bestmatch = NULL;
    it = trash_iterator(bin, 0);
    if(!it) {
        free((void*)filepath);
        free((void*)filetitle);
        return -1;
    }

    /* Iterator thru looking for matches. */
    while((cur = trashiterator_next(it))) {
        if(strcmp(filetitle, trashitem_getoriginalfiletitle(cur)) == 0 && strcmp(filepath, trashitem_getoriginalpath(cur)) == 0) {
            /* This is a matching file. */
            if(bestmatch == NULL) 
                bestmatch = cur;
            else {
                /* Already found another match - check which one is more recent. */
                if(trashitem_getdatedeletion(cur) > trashitem_getdatedeletion(bestmatch)) {
                    trashitem_free(bestmatch);
                    bestmatch = cur;
                }
            }
        } else
            trashitem_free(cur);
    }

    free((void*)filepath);
    free((void*)filetitle);
    /* Failed to find a match. */
    if(bestmatch == NULL) {
        report_error("Failed to find a matching file in trash");
        return -1;
    }

    ret  = trashitem_restore(bestmatch);
    trashitem_free(bestmatch);

    return ret;    
}

int trash_empty(trashbin_t *bin)
{
    trashiterator_t *it;
    trashitem_t *cur;

    it = trash_iterator(bin, TRUE);
    if(!it)
        return -1;

    /* Iterate thru and remove each item from trash. */
    while((cur = trashiterator_next(it))) {
        if(trashitem_removefromtrash(cur) != 0) {
            trashiterator_free(it);
            return -1;
        }
    }

    trashiterator_free(it);

    return 0;
}

trashiterator_t *trash_iterator(trashbin_t *bin, int release_kids)
{
    trashiterator_t *it;

    it = safe_alloc(sizeof(trashiterator_t));
    if(!it)
        return NULL;

    it->release_kids = release_kids;
    it->last_kid = NULL;
    
    /* Open up the directory for stepping thru. */
    it->dirlist = opendir(bin->dbpath);
    if(!it->dirlist) {
        report_error("Unable to open db directory for listing");
        free(it->dirlist);
    }
    it->bin = bin;

    return it;
}

void trashiterator_free
(trashiterator_t *iter)
{
    if(iter->release_kids && iter->last_kid != NULL)
        trashitem_free(iter->last_kid);

   closedir(iter->dirlist); 
    free(iter);
}

trashitem_t *trashiterator_next(trashiterator_t *iter)
{
    char *dbname;
    int ret;
    struct dirent *thisdirent;
    trashitem_t *item;

    /* First check if release_kids is true and if so free the last kid */
    if(iter->release_kids == TRUE && iter->last_kid != NULL) 
        trashitem_free(iter->last_kid);
        
    item = safe_alloc(sizeof(trashitem_t));
    if(!item)
         return NULL;
    
    iter->last_kid = NULL; /* Set last kid to NULL so that in case of failure a pointer is not left pointing to an invalid struct */
    
    /* Have allocated memory for trashitem now find next db entry and parse file */

    do {
        thisdirent = readdir(iter->dirlist);                                   
    } while(thisdirent != NULL && ((strcmp(thisdirent->d_name, ".") == 0) || (strcmp(thisdirent->d_name,  "..") == 0)));

    if(thisdirent == NULL) { /* No entries left. */
        return NULL;
    }

    /* Open and parse this file and file item contents. */
    dbname = safe_alloc(strlen(thisdirent->d_name) + 2 + strlen(iter->bin->dbpath));
    if(!dbname) {
        free(iter);
        return NULL;
    }
    
    strcpy(dbname, iter->bin->dbpath);
    strcat(dbname, "/");
    strcat(dbname, thisdirent->d_name);
 
    ret = readdb_entry(item, dbname);

    if(ret != 0) {
        free(iter);
        free(dbname);
        return NULL;    
    }

    /* Save the db name. */
    item->dbfilename = strdup(dbname);
    if(item->dbfilename == NULL) {
        report_error("Out of memory");
        free(iter);
        free(dbname);
        return NULL;        
    }
    /* Take the db out of the database name and it will point to the directory for accessing the file. */
    item->workingfilename = getworkingfilename(dbname);
    
    iter->last_kid = item;
    
    return item;
}

void trashitem_free(trashitem_t *item)
{
    free((void*)item->filetitle);
    free((void*)item->filepath);
    free((void*)item->workingfilename);
    free((void*)item->dbfilename);
    free((void*)item);
}


int trashitem_restore(trashitem_t *item)
{
    char *originalname;
    int ret;
    
    /* First move the item back to its original location in life. */
    originalname = safe_alloc(strlen(item->filepath) + strlen(item->filetitle) + 2);
    if(!originalname)
        return -1;
    strcpy(originalname, item->filepath);
    strcat(originalname, "/");
    strcat(originalname, item->filetitle);

    ret = move_file(item->workingfilename, originalname);
    if(ret != 0) {
        free(originalname);
        return -1;
    }
    
    /* File is moved back now restore to writeable - ignore failure - this is expected on symbolic link.  */
    if(!issymlink(originalname)) {
        ret = chmod(originalname, item->filemode);
        if(ret != 0)
            /* Better to keep going an remove db entry now that the file has been restored. */
            report_error("Unable to restore original mode");
    }
    free(originalname);
    
    /* File is safely moved back in file system - now remove db entry. */
    if(remove(item->dbfilename) != 0) {
        report_error("Unable to remove db entry");
        return -1;
    }

    return 0;
}


const char *trashitem_getworkingfilename(const trashitem_t *item)
{
    return item->workingfilename;
}

time_t trashitem_getdatedeletion(const trashitem_t *item)
{
    return item->datedeletion;
}


const char *trashitem_getoriginalpath(const trashitem_t *item)
{
    return item->filepath;
}

const char *trashitem_getoriginalfiletitle(const trashitem_t *item)
{
    return item->filetitle;
}

mode_t trashitem_getoriginalmode(const trashitem_t *item)
{
    return item->filemode;
}

int trashitem_removefromtrash(trashitem_t *item)
{
    char *buf;
    int retval;
    /* First remove the actual file. */
    /* Note: If this is a directory then its actually harder than it looks - must iterate thru and
     * changed everything to writable first (cannot remove a file inside a read-only directory. */
    /* Command looks like  chmod ugo+w -R filename */
    buf = safe_alloc(strlen(item->workingfilename) + 128);
    if(!buf)
        return -1;
    strcpy(buf, "chmod ugo+w -R ");
    strcat(buf, item->workingfilename);

    retval = system(buf);    
    if(retval != 0) {
        report_error("Error changing file to make deletable.");
        free(buf);
        return -1;
    }
    /* Okay now that it is deleteable remove it recursively
     * command looks like "rm -fR filename  */
    strcpy(buf, "rm -fR ");
    strcat(buf, item->workingfilename);
        
    if(system(buf) != 0) {
        report_error("Unable to remove working filename");
        free(buf);
        return -1;
    }
    free(buf);

    if(remove(item->dbfilename) != 0) {
        report_error("Unable to remove db filename");
        return -1;
    }

    return 0;
}

