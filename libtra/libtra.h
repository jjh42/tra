/***************************************************************************
                          libtra.h  -  description
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


#ifndef _LIBTRA_H
#define _LIBTRA_H

/* Need time types */
#ifndef time_t
#include <time.h>
#endif

#ifndef mode_t
#include <unistd.h>
#endif

#ifndef TRUE
#define TRUE        1
#endif
#ifndef FALSE
#define FALSE       0
#endif

struct trashbin_struct;
typedef struct trashbin_struct trashbin_t;


/** Initialize a trash bin. Must be done before using a trash bin. If NULL is passed as dir then the default directory will
 * be used (this is the usual behavior).
 *
 * Returns pointer to a trashbin on success, NULL on failure.
 *
 * @param dir   String pointer to a directory to use as the trashbin or NULL to use default.
 */
trashbin_t *trash_init(const char *dir) ;

/** Frees the resources associated with the trashbin. Trashbin is now invalid.
 *
 * Returns void
 *
 * @param trash     Trashbin to free.
 */
void trash_free(trashbin_t *trash);

/** Moves a file into the trashcan.This trashes a file with filename - if filename is not absolute
 * uses the cwd for the path.
 *
 * Returns 0 on success, negative on failure.
 *
 * @param bin   Trashbin to move filename into.
 * @param filename  The filename to trash.
 */
int trash_file(trashbin_t *bin, const char *filename);

/** Unlike trashitem_restore this restore accepts a filename and will iterate thru the list
 * looking for the most recent match to the filename (if not full filename it works from
 * the current directory) and then restores this file.
 *
 * Returns: 0 on sucess (ie file found and restore), negative on error.
 *
 * @param bin   Trashbin to search.
 * @param name  Filename to look for.
 */
int trash_restorefilename(trashbin_t *bin, const char *name);

/** Empties the trashbin of all the files it currently contains. Note: This will make all the files
 * in the trashbin much harder to recover.
 *
 * Returns: 0 on sucess, negative on error.
 *
 * @param bin Trashbin to empty
 */
int trash_empty(trashbin_t *bin);

/* Functions for iterating thru the trashed items. */
struct trashiterator_struct;
typedef struct trashiterator_struct trashiterator_t;

struct trashitem_struct;
typedef struct trashitem_struct trashitem_t;

/** Returns an iterator for use in cycling thru the trashitems. Must be freed with trash_freeiterator.
 * release_kids if set to true means that the trashitem_t * that the iterator gives on trashiterator_next
 * will not be released automatically on subsequent calls or on freeing the iterator.
 * The must be released by calling trash_freeiterator.
 *
 * Returns a pointer to a trashiterator_t or NULL on failure.
 *
 * @param bin   The trash bin
 */
trashiterator_t *trash_iterator(trashbin_t *bin, int release_kids);
void trashiterator_free(trashiterator_t *iter);

/** Gives the next trashitem in the trashcan.
 *
 * Returns the a pointer to the next item or NULL on failure.
 *
 * @param iter  A trashiterator used to iterate thru the trashcan.
 */
trashitem_t *trashiterator_next(trashiterator_t *iter);


/** Frees the memory associate with the trashitem item. Note: This makes any string returns be trashitem_get*
 * invalid. If release_kids was set to true in the trash_iterator then this function should no be called to free
 * its kids (they are freed automatically).
 *
 * Returns: void
 *
 * @param item  Pointer to a trashitem.
 */
void trashitem_free(trashitem_t *item);

/* Functions for extracting information from a trash item. */

/** Restores the trashitem to its original location in the filesystem.
 *  After this workingfilename will not be valid and further operations
 * on the file (ie restore an item twice) will cause an error.
 *
 * Returns 0 on success, negative on error.
 *
 * @param item Pointer a to a trashitem.
 */
int trashitem_restore(trashitem_t *item);

/** Returns a string giving the filename where a read-only copy of the delete file/directory etc. can be
 * found - this can be used to preview the file or get the size of the file etc. The pointer to the string returned
 * is only valid as long as the trashitem passed is valid.
 *
 * Returns NULL on failure or pointer to string.
 *
 * @param item  Pointer to a trashitem.
 */
const char * trashitem_getworkingfilename(const trashitem_t *item);

/** Returns in time_t the date the file was deleted (moved to the trash bin.
 *
 * Returns 0 on failure.
 *
 *@param item   Pointer to a trashitem.
 */
time_t trashitem_getdatedeletion(const trashitem_t *item);

/** Gives the full path not including the filename or trailing / where the deleted file
 * used to reside on disk. The pointer to the string returns is valid only as long as the
 * trashitem item is valid.
 *
 * Returns NULL on failure or pointer to a string.
 *
 *@param item   Pointer to a trash item.
 */
const char *trashitem_getoriginalpath(const trashitem_t *item);

/** Gives the filetitle (filename with no path included) or where the deleted file used to
 * reside on disk. Pointer to the string returned is valid only as long as the trashitem is valid.
 *
 * Returns NULL on failure or pointer to a string.
 *
 *@param item   Pointer to a trash item.
 */
const char *trashitem_getoriginalfiletitle(const trashitem_t *item);

/** Used to query the original access mode of the file before it was trashed (files in the trashbin are set
 * to readonly.
 *
 * Returns the access mode or 0 on error.
 *
 * @param item  Pointer to a trash item.
 */
mode_t trashitem_getoriginalmode(const trashitem_t *item);

/** Removes this file from the trashbin (deletes it for good). Be careful when using this function as it
 * means the file is now as irrecoverable as if it had been deleted with rm.
 *
 * Returns 0 on success, negative on error.
 *
 * @param item  Pointer to a trash item.
 */
int trashitem_removefromtrash(trashitem_t *item);
 
#endif

