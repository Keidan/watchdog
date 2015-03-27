/**
 *******************************************************************************
 * @file watchdog_xml.c
 * @author Keidan
 * @date 20/03/2015
 * @par Project
 * watchdog
 *
 * @par Copyright
 * Copyright 2015 Keidan, all right reserved
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY.
 *
 * Licence summary : 
 *    You can modify and redistribute the sources code and binaries.
 *    You can send me the bug-fix
 *
 * Term of the licence in in the file licence.txt.
 *
 *******************************************************************************
 */
#include "watchdog.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


#define __XMLCAST(str) ((const xmlChar*)str)

#define ALLOC_VALUE(ret, len, value, ptr) do {				\
    len = strlen((char*)value) + 1;					\
    ptr = malloc(len);							\
    if(!ptr) {								\
      ret = -1;								\
      fprintf(stderr,							\
	     "Unable to allocate the memory for the cfg file parser\n"); \
      goto leave;							\
    }									\
    bzero(ptr, len);							\
    strcpy(ptr, (char*)value);						\
  } while(0)




/**
 * @fn int watchdog_xml_load(const char* filename, struct watchdog_xml_s* process)
 * @brief Loads the XML file passed in parameter.
 * @param filename The path of the xml file.
 * @param process The output configuration.
 * @return 0 on success else -1.
 */
int watchdog_xml_load(const char* filename, struct watchdog_xml_s* process) {
  int ret = 0, len;
  xmlDocPtr doc = NULL;
  xmlNodePtr curL0 = NULL, curL1 = NULL, curL2 = NULL;
  xmlAttr* attribute = NULL;
  xmlChar* value = NULL;
  xmlChar* value2 = NULL;
  struct watchdog_xml_list_s *item;

  bzero(process, sizeof(struct watchdog_xml_s));

  FILE* t = fopen(filename, "rb");
  if(!t) {
    fprintf(stderr, "Unable to open the file %s: (%d) %s\n", filename, errno, strerror(errno));
    return -1;
  } else fclose(t);
  doc = xmlReadFile(filename, NULL, 0);
  if (doc == NULL) {
    fprintf(stderr, "Failed to parse %s\n", filename);
    return -1;
  }
  /* Place the current node to the root node */
  curL0 = xmlDocGetRootElement(doc);
  if (curL0 == NULL) {
    ret = -1;
    fprintf(stderr, "Empty document!\n");
    goto leave;
  }

  /* Validation step */
  if (xmlStrcmp(curL0->name, __XMLCAST("process"))) {
    ret = -1;
    fprintf(stderr, "document of the wrong type, root node != process\n");
    goto leave;
  }
  attribute = curL0->properties;
  while(attribute && attribute->name && attribute->children) {
    value = xmlNodeListGetString(curL0->doc, attribute->children, 1);
    if(!xmlStrcmp(attribute->name, __XMLCAST("name"))) {
      ALLOC_VALUE(ret, len, value, process->name);

      WD_ALLOC_NODE(item, struct watchdog_xml_list_s, "Unable to allocate the memory for the argument item", ret = -1; goto leave);
      ALLOC_VALUE(ret, len, value, item->value);
      WD_APPEND_NODE(process->args, item, process->args_count);
      process->args_count++;
    }
    xmlFree(value);
    attribute = attribute->next;
  }
  if(!process->name || !strlen(process->name)) {
    ret = -1;
    fprintf(stderr, "Invalid process name\n");
    goto leave;
  }

  curL1 = curL0->xmlChildrenNode;
  while (curL1 != NULL) {
    /* Traverse to obtain the desired node */
    if (!xmlStrcmp(curL1->name, __XMLCAST("path"))){
      value = xmlNodeGetContent(curL1);
      if(strlen((char*)value)) {
	len = strlen((char*)value) + strlen(process->name) + 2;
	process->path = malloc(len);
	if(!process->path) {
	  ret = -1;
	  fprintf(stderr,
		  "Unable to allocate the memory for the cfg file parser\n");
	  goto leave;
	}
	bzero(process->path, len);
	strcpy(process->path, (char*)value);
	len = strlen(process->path);
	if(process->path[len] != '/') process->path[len] = '/';
	strcat(process->path, process->name);
      }
      xmlFree(value);
    } else if (!xmlStrcmp(curL1->name, __XMLCAST("args"))){
      curL2 = curL1->xmlChildrenNode;
      while (curL2 != NULL) {
	if (!xmlStrcmp(curL2->name, __XMLCAST("arg"))){
	  value = xmlNodeGetContent(curL2);
	  if(strlen((char*)value)) {
	    WD_ALLOC_NODE(item, struct watchdog_xml_list_s, "Unable to allocate the memory for the argument item", ret = -1; goto leave);
	    ALLOC_VALUE(ret, len, value, item->value);
	    WD_APPEND_NODE(process->args, item, process->args_count);
	    process->args_count++;
	  }
	  xmlFree(value);
	}
	curL2 = curL2->next;
      }
    } else if (!xmlStrcmp(curL1->name, __XMLCAST("envs"))){
      curL2 = curL1->xmlChildrenNode;
      while (curL2 != NULL) {
	if (!xmlStrcmp(curL2->name, __XMLCAST("env"))){
	  value = value2 = NULL;
	  attribute = curL2->properties;
	  while(attribute && attribute->name && attribute->children) {
	    if(!xmlStrcmp(attribute->name, __XMLCAST("name"))) {
	      value = xmlNodeListGetString(curL2->doc, attribute->children, 1);
	    } else if(!xmlStrcmp(attribute->name, __XMLCAST("value"))) {
	      value2 = xmlNodeListGetString(curL2->doc, attribute->children, 1);
	    }
	    attribute = attribute->next;
	  }
	  if(value && value2) {
	    if(strlen((char*)value) && strlen((char*)value2)) {
	      WD_ALLOC_NODE(item, struct watchdog_xml_list_s, "Unable to allocate the memory for the evironment variable item", ret = -1; goto leave); 
	      len = strlen((char*)value) + strlen((char*)value2) + 2;
	      item->value = malloc(len);
	      if(!item->value) {
		ret = -1;
		fprintf(stderr,
			"Unable to allocate the memory for the cfg file parser\n");
		goto leave;
	      }
	      bzero(item->value, len);
	      strcpy(item->value, (char*)value);
	      strcat(item->value, "=");
	      strcat(item->value, (char*)value2);
	      WD_APPEND_NODE(process->envs, item, process->envs_count);
	      process->envs_count++;
	    }
	    if(value) xmlFree(value);
	    if(value2) xmlFree(value2);
	  } else {
	    if(value) xmlFree(value);
	    if(value2) xmlFree(value2);
	    ret = -1;
	    fprintf(stderr, "Unable to decode the environment variable\n");
	    goto leave;
	  }
	}
	curL2 = curL2->next;
      }
    }
    curL1 = curL1->next;
  }

leave:
  if(doc) xmlFreeDoc(doc);
  xmlCleanupParser();
  if(ret == -1) watchdog_xml_unload(process);
  return ret;
}

/**
 * @fn void watchdog_xml_unload(struct watchdog_xml_s* process)
 * @brief Release the resources allocated by the load function.
 * @param process The configuration.
 */
void watchdog_xml_unload(struct watchdog_xml_s* process) {
  struct watchdog_xml_list_s *cpy, *item;
  if(process) {
    if(process->name) free(process->name), process->name = NULL;
    if(process->path) free(process->path), process->path = NULL;
    
    cpy = process->args;
    while(cpy) {
      item = cpy;
      cpy = cpy->next;
      if(item) {
	if(item->value) free(item->value);
	free(item);
      }
    }
    process->args = NULL;
    cpy = process->envs;
    while(cpy) {
      item = cpy;
      cpy = cpy->next;
      if(item) {
	if(item->value) free(item->value);
	free(item);
      }
    }
    process->envs = NULL;
  }
}
