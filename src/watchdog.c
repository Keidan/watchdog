/**
 *******************************************************************************
 * @file watchdog.c
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
#include <signal.h>
#include <getopt.h>

pid_t child = -1;
char filename[MAX_FILENAME];
struct watchdog_xml_s xml;

static const struct option long_options[] = { 
    { "help"    , 0, NULL, 'h' },
    { "config"  , 1, NULL, 'c' },
    { "new"     , 0, NULL, 'n' },
    { NULL      , 0, NULL, 0   } 
};

static void m_exit(void);
static void sig_int(int s);
static void usage(int err);


int main(int argc, char** argv) {
  char c, **args = NULL, **envs = NULL;
  path_t path;
  FILE* f;
  struct sigaction sa;
  bzero(&xml, sizeof(struct watchdog_xml_s));
  atexit(m_exit);
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sig_int;
  (void)sigaction(SIGINT, &sa, NULL);
  (void)sigaction(SIGTERM, &sa, NULL);

  const char* bname = basename(argv[0]);

  /* init the default filename */
  bzero(path, MAX_FILENAME);
  bzero(filename, MAX_FILENAME);
  strcpy(path, CONFIG_FILE_FOLDER);
  if(path[strlen(path)] != '/')
    strcat(path, "/");
  strcpy(filename, path);
  strcat(filename, bname);
  strcat(filename, ".xml");
  /* parse the arguments */
  int opt;
  while ((opt = getopt_long(argc, argv, "hc:n", long_options, NULL)) != -1) {
    switch (opt) {
      case 'h': usage(0); break;
      case 'c': /* configuration file */
	strncpy(filename, optarg, MAX_FILENAME);
	break;
      case 'n':
	do {
	  f = fopen(filename, "wx");
	  if (!f && errno == EEXIST) {
	    printf("Are you sure you want to overwrite the existing file? (y/N):");
	    fscanf(stdin, "%c", &c);
	    if(c == 'y' || c == 'Y') {
	      unlink(filename);
	    } else return EXIT_FAILURE;
	  } else if(!f) {
	    fprintf(stderr, "Unable to create the file %s: (%d) %s\n", filename, errno, strerror(errno));
	    return EXIT_FAILURE;
	  } else break;
	} while(1);
	fwrite(EMPTY_CONFIG_FILE, 1, strlen(EMPTY_CONFIG_FILE), f);
	fclose(f);
	return EXIT_SUCCESS;
      default: /* '?' */
	fprintf(stderr, "Unknown option '%c'\n", opt);
	usage(EXIT_FAILURE);
	break;
    }
  }
  
  /* load the config file */
  if(watchdog_xml_load(filename, &xml))
    return EXIT_FAILURE;

  watchdog_utils_conver_to_array(xml.args, xml.args_count, &args);
  watchdog_utils_conver_to_array(xml.envs, xml.envs_count, &envs);
  
  watchdog_respawn(xml.path, args, envs);
  if(args) free(args);
  if(envs) free(envs);

  return EXIT_SUCCESS;
}

static void usage(int err) {
  fprintf(stdout, "usage: watchdog options\n");
  fprintf(stdout, "\t--help, -h: Print this help.\n");
  fprintf(stdout, "\t--config, -c: Load a config file.\n");
  fprintf(stdout, "\t--new, -n: Create a new config file.\n");
  fprintf(stdout, "\tIf no configuration file is passed as parameter.\n");
  fprintf(stdout, "\tThe application search a configuration file localized into the folder %s and named: <the_application_name>.xml\n", CONFIG_FILE_FOLDER);
  fprintf(stdout, "\tIt's possible to create symbolic links with several configuration files:\n");
  fprintf(stdout, "\tln -s watchdog watchdog-foo\n");
  fprintf(stdout, "\twatchdog-foo -n\n");
  fprintf(stdout, "\tnow you can edit the config file %s/watchdog-foo\n", CONFIG_FILE_FOLDER);
  exit(err);
}

static void m_exit(void) {
  if(child != -1) {
    kill(child, SIGTERM);
    child = -1;
  }
  watchdog_xml_unload(&xml);
}
static void sig_int(int s) { exit(s); }
