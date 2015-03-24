/**
 *******************************************************************************
 * @file watchdog.h
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
#ifndef __WATCHDOG_H__
  #define __WATCHDOG_H__

  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <errno.h>
  #include <string.h>
  #include <libgen.h>

#define EMPTY_CONFIG_FILE "\
<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<!-- The process name eg with /bin/ls name=\"ls\" -->\n\
<process name=\"foo\">\n\
  <!-- Optional, if the file is not in the PATH -->\n\
  <path></path>\n\
  <args>\n\
    <!-- Optional, arguments list -->\n\
    <arg></arg>\n\
  </args>\n\
  <envs>\n\
    <!-- Optional, environment variables list -->\n\
    <env name=\"\" value=\"\"/>\n\
  </envs>\n\
</process>\n"

  #ifndef CONFIG_FILE_FOLDER
    #define CONFIG_FILE_FOLDER "/etc"
  #endif

  #define WD_ALLOC_NODE(node, type, err, goto_lbl) do{	\
    node = malloc(sizeof(type));			\
    if(!node) {						\
      ret = -1;						\
      fprintf(stderr, err);				\
      goto goto_lbl;					\
    }							\
    memset(node, 0, sizeof(type));			\
  } while(0)

  #define WD_APPEND_NODE(root, node, idx) do {	\
    node->index = idx;				\
    if(!root)					\
      root = node;				\
    else {					\
      node->next = root->next;			\
      root->next = node;			\
    }						\
  } while(0)



  /****************
   * XML
   ****************/
  struct watchdog_xml_list_s {
      char* value;
      int index;
      struct watchdog_xml_list_s *next;
  };
  struct watchdog_xml_s {
      char* name;
      char* path;
      struct watchdog_xml_list_s *args;
      int args_count;
      struct watchdog_xml_list_s *envs;
      int envs_count;
  };

  /**
   * @fn int watchdog_xml_load(const char* filename, struct watchdog_xml_s* process)
   * @brief Loads the XML file passed in parameter.
   * @param filename The path of the xml file.
   * @param process The output configuration.
   * @return 0 on success else -1.
   */
  int watchdog_xml_load(const char* filename, struct watchdog_xml_s* process);

  /**
   * @fn void watchdog_xml_unload(struct watchdog_xml_s* process)
   * @brief Release the resources allocated by the load function.
   * @param process The configuration.
   */
  void watchdog_xml_unload(struct watchdog_xml_s* process);

  /****************
   * UTILS
   ****************/
  #define MAX_FILENAME 255

  typedef char path_t[MAX_FILENAME];

  /**
   * @fn  int watchdog_utils_find_file(const char* name, path_t path)
   * @brief Find a file into the system PATH.
   * @param name The filename.
   * @param path The output path with the filename.
   * @return 0 on success else -1.
   */
  int watchdog_utils_find_file(const char* name, path_t path);

  /**
   * @fn int watchdog_utils_strindexof(const char* source, const char* needed)
   * @brief Get the index of a string.
   * @param source The src string.
   * @param needed The needed string.
   * @return -1 if not found else the index into the source string.
   */
  int watchdog_utils_strindexof(const char* source, const char* needed);

  /**
   * @fn int watchdog_utils_complete_env(struct watchdog_xml_s* process)
   * @brief Concate the process envs with the system envs.
   * @param process The process context.
   * @return -1 ont failure else 0
   */
  int watchdog_utils_complete_env(struct watchdog_xml_s* process);

  /**
   * @fn void watchdog_utils_conver_to_array(struct watchdog_xml_list_s *root, int count, char ***array)
   * @brief Convert the linkedlist to a char**
   * @param root The root point of the linkedlist.
   * @param count Linkedlist items
   * @param array The output array.
   */
  void watchdog_utils_conver_to_array(struct watchdog_xml_list_s *root, int count, char ***array);

  /**
   * @fn void watchdog_utils_sort_list(struct watchdog_xml_list_s** root)
   * @brief Sort a linkedlist in ascending mode.
   * @param root A pointer to the linkedlist.
   */
  void watchdog_utils_sort_list(struct watchdog_xml_list_s** root);

  /****************
   * RUN
   ****************/
  typedef enum { WD_SPAWN_ERROR, WD_SPAWN_PARENT, WD_SPAWN_CHILD } watchdog_spawn_result_et;

  /**
   * @fn watchdog_spawn_result_et watchdog_spawn(char* name, char** args, char** envs)
   * @brief Start a process and wait for the exit.
   * @param name The process name.
   * @param args The process args.
   * @param envs The process envs.
   * @return The result code.
   */
  watchdog_spawn_result_et watchdog_spawn(char* name, char** args, char** envs);

  /**
   * @fn watchdog_spawn_result_et watchdog_respawn(char* name, char** args, char** envs)
   * @brief Start a process and restart if the process exit.
   * @param name The process name.
   * @param args The process args.
   * @param envs The process envs.
   * @return The result code.
   */
  watchdog_spawn_result_et watchdog_respawn(char* name, char** args, char** envs);


#endif /* __WATCHDOG_H__ */
