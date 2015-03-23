/**
 *******************************************************************************
 * @file watchdog_utils.c
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
#include <stdint.h>

extern char **environ;


/**
 * @fn int watchdog_utils_strindexof(const char* source, const char* needed)
 * @brief Get the index of a string.
 * @param source The src string.
 * @param needed The needed string.
 * @return -1 if not found else the index into the source string.
 */
int watchdog_utils_strindexof(const char* source, const char* needed) {
  char * found = strstr(source, needed);
  if(found != NULL) return found - source;
  return -1;
}

/**
 * @fn  int watchdog_utils_find_file(const char* name, path_t path)
 * @brief Find a file into the system PATH.
 * @param name The filename.
 * @param path The output path with the filename.
 * @return 0 on success else -1.
 */
int watchdog_utils_find_file(const char* name, path_t path) {
  const char* paths = getenv ("PATH");
  char* works, *tmp, *copy;
  uint32_t l;
  FILE* f;
  bzero(path, sizeof(path_t));
  if(!paths) {
    fprintf(stderr, "Null env PATH.");
    return -1;
  }
  works = strdup(paths);
  copy = works;
  while((tmp = strsep(&works, ":")) != NULL) {
    strcpy(path, tmp);
    l = strlen(path);
    if(path[l] != '/') path[l] = '/';
    strcat(path, name);
    f = fopen(path, "rb");
    if(!f) bzero(path, sizeof(path_t));
    else {
      fclose(f);
      break;
    }
  }
  free(copy);
  if(!strlen(path)) {
    return -1;
  }
  return 0;
}

/**
 * @fn 
 * @brief Concate the process envs with the system envs.
 * @param process The process context.
 * @return -1 ont failure else 0
 */
int watchdog_utils_complete_env(struct watchdog_xml_s* process) {
  int idx, len, ret = 0;;
  _Bool found;
  char **c_env, *env;
  struct watchdog_xml_list_s *item, *cpy, *it;
  for (c_env = environ; *c_env; ++c_env) {
    env = *c_env;
    found = 0;
    if(process->envs) {
      cpy = process->envs;
      while(cpy) {
	item = cpy;
	cpy = cpy->next;
	idx = watchdog_utils_strindexof(item->value, "=");
	if(!strncmp(item->value, env, idx)) {
	  found = 1;
	  break;
	}
      }
    }
    if(!found) {
      WD_ALLOC_NODE(it, struct watchdog_xml_list_s, "Unable to allocate the memory for the argument item", leave);
      len = strlen(env) + 1;
      it->value = malloc(len);
      if(!it->value) {
	free(it);
	ret = -1;
	fprintf(stderr,
	       "Unable to allocate the memory for the cfg file parser\n");
	goto leave;
      }
      bzero(it->value, len);
      strcpy(it->value, env);
      WD_APPEND_NODE(process->envs, it);
    }
  }
leave:
  return ret;
}

/**
 * @fn void watchdog_utils_conver_to_array(struct watchdog_xml_list_s *root, int count, char ***array)
 * @brief Convert the linkedlist to a char**
 * @param root The root point of the linkedlist.
 * @param count Linkedlist items
 * @param array The output array.
 */
void watchdog_utils_conver_to_array(struct watchdog_xml_list_s *root, int count, char ***array) {
  struct watchdog_xml_list_s *cpy, *it;
  int i = 0;
  if(!root) {
    *array = NULL;
    return;
  }
  *array = malloc(count + 1);
  bzero(*array, count + 1);
  cpy = root;
  while(cpy) {
    it = cpy;
    cpy = cpy->next;
    (*array)[i++] = it->value;
  }
  (*array)[i] = NULL;
}

