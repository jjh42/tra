
bin_PROGRAMS = tra


tra_SOURCES = main.c 
tra_LDADD   = $(top_srcdir)/libtra/libtra.la 

SUBDIRS = docs 

EXTRA_DIST = main.c 

####### kdevelop will overwrite this part!!! (end)############
# set the include path found by configure
INCLUDES= $(all_includes)  -I$(top_srcdir)/libtra

# the library search path.
tra_LDFLAGS = $(all_libraries) 
                                                                                
