/***************************************************************************
                          xml.c  -  For working with xml files
                             -------------------
    begin                : Thu Dec 25 2003
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
#include <libxml/tree.h>
#include <string.h>

#include "libtra_internal.h"

#define EXTERNAL_ID "-//ME.NET.NZ/DTD trashinfo 1.0//EN"

int createdb_entry(const char *dbname, trashitem_t *trashdata)
{
      /* Create the xml document describing file */
      xmlDocPtr doc;
      xmlNodePtr root;
      xmlNodePtr item;
      char datebuf[256];
      char modebuf[256];

      doc = xmlNewDoc((xmlChar*)"1.0");
      if(!doc) {
          report_error("Unable to create xml doc");
          return -1;
      }

   	 if(!xmlNewDocProp(doc, (xmlChar*)"encoding", (xmlChar*)"UTF-8")) {
      	report_error("Unable to make encoding");
       	return -1;
      }

      xmlCreateIntSubset (doc, (xmlChar*) "trashinfo", (xmlChar*) EXTERNAL_ID, (xmlChar *)"http://www.me.net.nz/DTD/trashinfo-1.0.dtd");
                                                                                  	
      /* Add nodes to the xml doc */
      root = xmlNewDocNode (doc, NULL, (xmlChar*)"trashinfo", NULL);
      if(!root) {
          report_error("Unable to make a root element");
      }
	xmlDocSetRootElement(doc, root);
      xmlNewProp(root, (xmlChar*) "xmlns", (xmlChar*) "http://www.me.net.nz/DTD/trashinfo");
	
	item = xmlNewChild(root, NULL, (xmlChar*) "item", NULL);


      /* Add the attributes */
      snprintf(datebuf, sizeof(datebuf), "%x", (unsigned int) trashdata->datedeletion);
      snprintf(modebuf, sizeof(modebuf), "%o", (unsigned int) trashdata->filemode);
      /* First save the datedeletion attribute */
	xmlNewProp(item, (const xmlChar*) "filetitle", (const xmlChar*)trashdata->filetitle);
	xmlNewProp(item, (const xmlChar*) "filepath", (const xmlChar*)trashdata->filepath);
	xmlNewProp(item, (const xmlChar*)  "datedeletion", (const xmlChar*) datebuf);
      xmlNewProp(item, (const xmlChar*) "mode", (const xmlChar*) modebuf);
    
    	if(xmlSaveFormatFile(dbname, doc,1 ) == -1) {
          report_error("Error saving xml file");
          return -1;
    	}

     xmlFreeDoc(doc);

    return 0;
}

int readdb_entry(trashitem_t *trashdata, const char *dbfile)
{
    /* First read in data from xml file. */
    xmlDocPtr doc;
    xmlDtdPtr xmldtd;
    xmlNodePtr root;
    xmlNodePtr item;
    xmlChar *data;
    unsigned int timed;
    unsigned int mode;

    doc = xmlParseFile(dbfile);
    if(!doc) {
        report_error("Unable to parse xml doc");
        return -1;
    }
    /* First check it is a valid xml file that we can parse. */
    xmldtd = xmlGetIntSubset(doc);
    if(!xmldtd) {
        xmlFreeDoc(doc);
        report_error("Unable to get internal subset.");
        return -1;
    }
    if(xmlStrcmp(xmldtd->ExternalID, (const xmlChar*)EXTERNAL_ID) != 0) {
        xmlFreeDoc(doc);
        report_error("Mismatching xml id's - possibly newer version of trashbin needed");
        return -1;
    } 
 
    /* Now traverse down the file item entry */
    root = xmlDocGetRootElement (doc);
    item = root->xmlChildrenNode;
    while(xmlStrcmp(item->name, (const  xmlChar*) "item") != 0) {
                
        item = item->next;
        if(item == NULL) {
            xmlFreeDoc(doc);
            report_error("Unable to find item element");
            return -1;
        }
    }
                
    data = xmlGetProp(item, (const xmlChar *) "filetitle");
    trashdata->filetitle = strdup((const char*)data);
    xmlFree(data);
    data = xmlGetProp(item, (const xmlChar *) "filepath");
    trashdata->filepath = strdup((const char*)data);
    xmlFree(data);
    data = xmlGetProp(item, (const xmlChar *) "datedeletion");
    /* Have the data convert to time_t and store */
    sscanf((const char *)data, "%x", &timed);
    trashdata->datedeletion = (time_t) timed; 
    xmlFree(data);
    /* Get the mode */
    data = xmlGetProp(item, (const xmlChar *) "mode");
    sscanf((const char *)data, "%o", &mode);
    trashdata->filemode = (mode_t) mode;
    xmlFree(data);
    
    xmlFreeDoc(doc);

    return 0;
}

