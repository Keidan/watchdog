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

path_t pidfile;
pid_t pid = -1;
pid_t child = -1;
char filename[MAX_FILENAME];
struct watchdog_xml_s xml;



static const struct option long_options[] = { 
    { "help"       , 0, NULL, 'h' },
    { "config"     , 1, NULL, 'c' },
    { "directory"  , 1, NULL, 'd' },
    { "new"        , 0, NULL, 'z' },
    { "path"       , 1, NULL, 'p' },
    { "name"       , 1, NULL, 'n' },
    { "arg"        , 1, NULL, 'a' },
    { "env"        , 1, NULL, 'e' },
    { "pid"        , 0, NULL, '0' },
    { "pidfile"    , 1, NULL, '1' },
    { NULL         , 0, NULL, 0   } 
};

static void m_exit(void);
static void sig_int(int s);
static void usage(const char* name, int err);


int main(int argc, char** argv) {
  char c, **args = NULL, **envs = NULL;
  _Bool z = 0;
  path_t path, cmd_path;
  FILE* f;
  struct sigaction sa;
  struct watchdog_xml_list_s *item;
  

  bzero(pidfile, sizeof(path_t));
  bzero(&xml, sizeof(struct watchdog_xml_s));
  atexit(m_exit);
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sig_int;
  (void)sigaction(SIGINT, &sa, NULL);
  (void)sigaction(SIGTERM, &sa, NULL);

  const char* bname = basename(argv[0]);
  bzero(path, MAX_FILENAME);
  bzero(filename, MAX_FILENAME);
  bzero(cmd_path, MAX_FILENAME);
  xml.args_count = 1; /* anticipation for the name field */

  /* parse the arguments */
  int opt;
  while ((opt = getopt_long(argc, argv, "hc:zd:n:a:e:p:01:", long_options, NULL)) != -1) {
    switch (opt) {
      case 'h': usage(bname, 0); break;
      case 'c': /* configuration file */
	strncpy(filename, optarg, MAX_FILENAME);
	break;
      case 'd': /* directory */
	strncpy(path, optarg, MAX_FILENAME);
	break;
      case 'z': /* new */
	z = 1;
	break;
      case '0': /* pid */
	pid = getpid();
	break;
      case '1': /* pidfile */
	strcpy(pidfile, optarg);
	break;
      case 'p': /* path */
	WD_STRALLOCCPY(xml.path, optarg, return EXIT_FAILURE);
	break;
      case 'n': /* name */
	WD_STRALLOCCPY(xml.name, optarg, return EXIT_FAILURE);
	WD_ALLOC_NODE(item, struct watchdog_xml_list_s, "Unable to allocate the memory for the argument item", return EXIT_FAILURE);
	WD_STRALLOCCPY(item->value, optarg, return EXIT_FAILURE);
	WD_APPEND_NODE(xml.args, item, xml.args_count);
	xml.args_count++;
	break;
      case 'a': /* arg */
	WD_ALLOC_NODE(item, struct watchdog_xml_list_s, "Unable to allocate the memory for the argument item", return EXIT_FAILURE);
	WD_STRALLOCCPY(item->value, optarg, return EXIT_FAILURE);
	WD_APPEND_NODE(xml.args, item, xml.args_count);
	xml.args_count++;
	break;
      case 'e': /* env */
	WD_ALLOC_NODE(item, struct watchdog_xml_list_s, "Unable to allocate the memory for the argument item", return EXIT_FAILURE);
	WD_STRALLOCCPY(item->value, optarg, return EXIT_FAILURE);
	WD_APPEND_NODE(xml.envs, item, xml.envs_count);
	xml.envs_count++;
	break;
      default: /* '?' */
	fprintf(stderr, "Unknown option '%c'\n", opt);
	usage(bname, EXIT_FAILURE);
	break;
    }
  }
  if(strlen(pidfile) == 0)  sprintf(pidfile, "%s/%s.pid", PID_FOLDER, bname);
  if(pid > 0) {
    FILE *fpid = fopen(pidfile, "r");
    if(fpid) {
      fclose(fpid);
      fprintf(stderr, "The pid file '%s' already exists!\n", pidfile);   
      usage(bname, EXIT_FAILURE);   
    }
    fpid = fopen(pidfile, "w+");
    if(fpid) {
      fprintf(fpid, "%d\n", pid);
      fclose(fpid);
    } else 
      fprintf(stderr, "Unable to create the pid file '%s': (%d) %s\n", pidfile, errno, strerror(errno));
  }
  
  if(!xml.name) {
    /* init the default filename */
    if(!strlen(path))
      strcpy(path, CONFIG_FILE_FOLDER);
    if(path[strlen(path) - 1] != '/')
      strcat(path, "/");
    strcpy(filename, path);
    strcat(filename, bname);
    strcat(filename, ".xml");

    if(z) {
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
    }
  
    /* load the config file */
    if(watchdog_xml_load(filename, &xml))
      return EXIT_FAILURE;
  }
  /* prepare the path and find the binary */
  if(!xml.path || !strlen(xml.path)) {
    path_t p;
    if(watchdog_utils_find_file(xml.name, p)) {
      fprintf(stderr, "The file '%s' was not found.\n", xml.name);
      return EXIT_FAILURE;
    }
    WD_STRALLOCCPY(xml.path, p, return EXIT_FAILURE);
  }
  
  /* add the parent env variables */
  watchdog_utils_complete_env(&xml);
  /* sort the arguments */
  watchdog_utils_sort_list(&xml.args);

  /* convert linkedlist to char** */
  watchdog_utils_conver_to_array(xml.args, xml.args_count, &args);
  watchdog_utils_conver_to_array(xml.envs, xml.envs_count, &envs);
  
  
  strcpy(cmd_path, xml.path);
  if(cmd_path[strlen(cmd_path) - 1] != '/')
    strcat(cmd_path, "/");
  strcat(cmd_path, args[0]);

  watchdog_respawn(cmd_path, args, envs);
  if(args) free(args);
  if(envs) free(envs);

  return EXIT_SUCCESS;
}

static void usage(const char* name, int err) {
  fprintf(stdout, "usage: watchdog options\n");
  fprintf(stdout, "\t--help, -h: Print this help.\n");
  fprintf(stdout, "\t--pid: Write the pid into the file pointed by pidfile.\n");
  fprintf(stdout, "\t--pidfile: The file containing the pid (default: %s/%s.pid)\n", PID_FOLDER, name);
  fprintf(stdout, "Mode standalone:\n");
  fprintf(stdout, "\t--path, -p: The process path (optional if the binary is in the PATH).\n");
  fprintf(stdout, "\t--name, -n: The process name.\n");
  fprintf(stdout, "\t--arg, -a: The process argument (repeat for more).\n");
  fprintf(stdout, "\t--env, -e: The process environment variable (repeat for more).\n");
  fprintf(stdout, "Mode file:\n");
  fprintf(stdout, "\t--directory, -d: The directory to search the configuration file (default:%s).\n", CONFIG_FILE_FOLDER);
  fprintf(stdout, "\t--config, -c: Load a config file.\n");
  fprintf(stdout, "\t--new, -z: Create a new config file.\n");
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
  if(pid > 0) {
    pid_t _pid;
    FILE *fpid = fopen(pidfile, "r");
    if(fpid) {
      fscanf(fpid, "%d", &_pid);
      fclose(fpid);
      if(_pid == pid) unlink(pidfile);
    }
  }
}
static void sig_int(int s) { exit(s); }
