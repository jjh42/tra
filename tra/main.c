/***************************************************************************
                          main.c  -  description
                             -------------------
    begin                : Wed Dec 24 09:39:09 NZDT 2003
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <strings.h>

#include <libtra.h>
#include <libtra_internal.h>

/** Accepts an initialized trash bin and lists to stdout all the files contained in the trash bin.
 *
 * Returns 0 on success, negative on failure.
 *
 * @param bin Trashbin to list
 */
int list_trash(trashbin_t *bin);

/** Empties the trashbin (after first checking with the user that this is really what they want.
 *
 * Returns 0 on success, negative on failure.
 *
 * @param bin Trashbin to empty
 */
int empty_trash(trashbin_t *bin);

const char *usage =
"[switches] filenames\n"
"Use --help to get more options\n";

const char *help =
"-e --empty        empty the files in the trash permanetly\n"
"-l --list			the files contained in the trash\n"
"-r --restore       restore the files named after\n"
"-t  --trashloc	set the trash location\n"
"-H --help		the menu\n"
"-V --version 	print the version number\n";

#if !defined(HAVE_CONFIG_H)
#define VERSION		"Unknown"
#endif

#define ARG_STDIO	254
const char *opts = "eVHlrt:";
struct option lopts[] =
{
      {"empty", no_argument, NULL, 'e'},
	{"version", no_argument, NULL, 'V'},
	{"help", no_argument, NULL, 'H'},
	{"trashloc", required_argument, NULL, 't'},
	{"restore", no_argument, NULL, 'r'},
	{"list",  no_argument, NULL, 'l'},	
	{NULL, no_argument, NULL, 0}
};

enum Tasks
{
    Task_List = 1,
    Task_Restore = 2,
    Task_Trash = 3,
    Task_Empty = 4
};

int main(int argc, char *argv[])
{
    char *programname;
    enum Tasks task;
    const char *trashlocation;
    int retval;
    trashbin_t *trashbin;
    char c;

    task = Task_Trash; /* Default is to trash files */

    trashlocation = NULL;
    /* First thing we check our command line. */
    /* Set our program name. */
    programname = argv[0];

    /* Read our arguments. */
    while((c = getopt_long(argc, argv, opts, lopts, NULL)) != EOF)
        switch(c) {
  		case 'V':
  			fprintf(stderr, "%s : %s\n", programname, VERSION);
  			return 0;
  		case 'H':
  			/* Print our help string. */
  			fprintf(stderr, help);
  			return 0;
            case 'r':
                task = Task_Restore;
                break;
            case 'e':
                task = Task_Empty;
                break;
		case 'l':        
                task = Task_List;
                break;
            case 't':
                trashlocation = optarg;
                break;
  		default:
  			/* This was an invalid option. */
                  fprintf(stderr, "Invalid option %s\n", argv[optind]);
  			fprintf(stderr, "Usage %s %s", programname, usage);
  			return -1;
  	}

        trashbin = trash_init(trashlocation);
        if(!trashbin) {
            return -1;
        }
   
        switch(task) {
            case Task_List:
                 retval = list_trash(trashbin);
                 trash_free(trashbin);
                 return retval;
            case Task_Empty:
                retval = empty_trash(trashbin);
                trash_free(trashbin);
                return retval;
            default:
                break;
        };
        

        /* Either restoring or trashing the list of files */
        for(; optind < argc; optind ++) {
            if(task == Task_Restore)
                retval = trash_restorefilename(trashbin, argv[optind]);
            else
                retval = trash_file(trashbin, argv[optind]);
                
            if(retval != 0) {
                trash_free(trashbin);
                return retval;
            }                    
        }

        trash_free(trashbin);

        return 0;
}


int list_trash(trashbin_t *bin)
{
    trashiterator_t *iter;
    trashitem_t *item;
    time_t dd;
    
    iter = trash_iterator(bin, TRUE);
    if(!iter)
        return -1;

    printf("Listing trash bin\nTitle    Path    Date Deletion\n---------------------------\n");
    /* Iterate thru and print any kids out */
    while((item = trashiterator_next(iter))) {
        /* Have a working item print results */
        dd = trashitem_getdatedeletion(item);
        printf("%s\t%s\t%s\n", trashitem_getoriginalfiletitle(item), trashitem_getoriginalpath(item),
            ctime(&dd));
    }

    trashiterator_free(iter);

    return 0;    
}

int empty_trash(trashbin_t *bin)
{
    char buf[4];
    
    fprintf(stderr, "Are you sure you want to empty the trash bin (yes, no)\n");

    if(!fgets(buf, sizeof(buf), stdin)) {
        fprintf(stderr, "Unable to get user input\n");
        return -1;
    }

    if(strncasecmp(buf, "no", 2) == 0)
        return 0;

    if(strncasecmp(buf, "yes", 3) != 0) {
        fprintf(stderr, "Unconfirmed user input: type 'yes' to allow emptying\n");
        return -1;
    }

    return trash_empty(bin);
}
