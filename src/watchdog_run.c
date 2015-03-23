/**
 *******************************************************************************
 * @file watchdog_run.c
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
#include <sys/types.h>
#include <sys/wait.h>

extern pid_t child;

/**
 * @fn watchdog_spawn_result_et watchdog_spawn(char* name, char** args, char** envs)
 * @brief Start a process and wait for the exit.
 * @param name The process name.
 * @param args The process args.
 * @param envs The process envs.
 * @return The result code.
 */
watchdog_spawn_result_et watchdog_spawn(char* name, char** args, char** envs) {
  int status;
  pid_t w;
  if((child = fork()) >= 0) {// fork was successful
    if(child == 0) {// child process
      if(execve(name, args, envs) == -1) {
	fprintf(stderr, "Unable to starts the process '%s': (%d) %s.\n", args[0], errno, strerror(errno));
      }
      return WD_SPAWN_CHILD;
    }
    else {//Parent process
      w = waitpid(child, &status, WUNTRACED | WCONTINUED);
      if (w == -1) {
	fprintf(stderr, "Unable to wait for the process '%s[%d]' : (%d) %s.\n", args[0], child, errno, strerror(errno));
	exit(EXIT_FAILURE);
      }
      //printf("Child '%s:%d' leave with termsig %d.\n", args[0], child, WTERMSIG(status));
      //logger(LOG_INFO, "Child '%s[%d]' leave with termsig %d.\n", args[0], child, WTERMSIG(status));
      child = 0;
    }
  } else// fork failed
    return WD_SPAWN_ERROR;
  return WD_SPAWN_PARENT;
}

/**
 * @fn watchdog_spawn_result_et watchdog_respawn(char* name, char** args, char** envs)
 * @brief Start a process and restart if the process exit.
 * @param name The process name.
 * @param args The process args.
 * @param envs The process envs.
 * @return The result code.
 */
watchdog_spawn_result_et watchdog_respawn(char* name, char** args, char** envs) {
  watchdog_spawn_result_et result;
  do {
    result = watchdog_spawn(name, args, envs);
  } while(result == WD_SPAWN_PARENT);
  return result;
}
