/***************************************************************************
                          libtra_internal.h  -  description
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

#ifndef _LIBTRA_INTERNAL_H
#define _LIBTRA_INTERNAL_H

#include <libtra.h>
#include <dirent.h>

/* Append to $HOME to locate the trash folder */
#define TRASH_FOLDER_EXT    "/.trash"
#define DB_FOLDER_EXT    "/db"

#define CREATE_DIR_MODE 0777
 
struct trashbin_struct
{
    char *path;
    char *dbpath;
};

struct trashiterator_struct
{
    int release_kids;
    DIR *dirlist;
    trashitem_t *last_kid;
    trashbin_t *bin;
};

struct trashitem_struct
{
  	char *filetitle;
	char *filepath;
      char *dbfilename;
      char *workingfilename;
      time_t datedeletion;
      mode_t filemode;
};


/* Internal functions */
#define safe_alloc(s) _safe_alloc(s, __LINE__, __FILE__)
/* Just like malloc only it reports an error on failure to allocate memory.
 *
 * Returns pointer to allocated memory or NULL on failure.
 *
 * @param   size    Size of area to allocate.
 * @param   line    Linenumber to report error from.
 * @param   file    Filename where error occured.
 */
void *_safe_alloc(size_t size, int line, const char *file);
                                     
/* report_error is called to report an error - prints it out to the command line */
#define report_error(err) _report_error(err, __LINE__, __FILE__)
/** Helper function used my macro report_error the prints errors to stderr.
 *
 * Returns void.
 *
 * @param err   A string describing the error.
 * @param line  Line number of error.
 * @param file  Filename error occured in.
 */
void _report_error(const char *err, int line, const char *file);

/* Move a file (technically a node - can be file, directory, symlink etc. Returns 0 on success */

/** Move the node (file, dir, symlink etc) from oldname to newname.
 *
 * Returns 0 on success, negative on failure.
 *
 * @param oldname   The name of the filename currently.
 * @param newname  Filename to move it to.
 */
int move_file(const char *oldname, const char *newname);

/* Set a file to be readonly while preserving all the other attributes.
 * Note: Ignores failure of chmod because chmod fails on symlinks (symlinks mode
 * cannot be changed).
 *
 * Returns 0 on success, negative on failure.
 *
 * @param   newname      The filename to operate on.
 */
int set_readonly(const char *newname);

/** Used in debugging to assert as best a possible that the trashbin is valid.
 *
 * Returns: void
 *
 * @param bin   The trashbin to check.
 */
void assert_trashvalid(trashbin_t *bin);

/** Create a database file at dbname containing all the important info about dbname before it is
 * moved into db. Does not check the actual file just uses the trashdata.
 *
 * Returns 0 on success, negative on error.
 *
 * @param dbname    The filename of the file in which to store the xml data.
 * @param trashdata  Contains info about the file which is saved.
 */
int createdb_entry(const char *dbname, trashitem_t *trashdata);

/** Reads the data from the db filename db file into trashdata (allocating filetitle and filepath).
 *
 * Returns: 0 on success, negative on error
 *
 * @param trashdata Must be a pointer to an already allocated trashdata_struct that will be filled with data.
 * @param dbfile      A string giving the filename of the dbfile to open
 */
int readdb_entry(trashitem_t *trashdata, const char *dbfile);

/** Get the filetitle and absolute filepath of the given filename - using the cwd if necessary and removing any double // etc
 * so that this is a unique filename.. Note: the pointers need to be freed when they are finished with.
 *
 * Returns 0 on success, negative on error.
 *
 * @param filename   Filename to make absolute and get filepath and filetitle from.
 * @param filepath    Pointer to set to filepath.
 * @param filetitle     Pointer to set to filetitle.
 */
int gettitleandpath(const char *filename, char **filepath, char **filetitle);

/** This function takes the string filename of the db file and modifies it to point to a working copy of the trashed file.
 * It then returns a pointer to this string.
 * ie Passed "/home/test/.trash/db/asdf" and modifies string to "/home/test/.trash/asdf" and returns a pointer to this string.
 *
 * Returns: Pointer to working copy of the trashed file or NULL on failure.
 *
 * @param dbname    Pointer to a modifiable string of the db file.
 */
char *getworkingfilename(char *dbname);

/** Helper function that does a lstat of filename and returns the current mode of the file.
 *
 * Returns: The mode of the file.
 *
 * @param filename  The filename to stat.
 */
mode_t getmode(const char *filename);

/** Checks by calling lstat to see what type of file this is to see if it is a symlink.
 *
 * Returns: 0 if file is not a symlink otherwise 1.
 *
 * @param filename. The filename to test for being a symlink.
 */
int issymlink(const char *filename);

#endif /*_LIBTRA_INTERNAL_H */


