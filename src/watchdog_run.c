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


#define MINIMUM_RESPAWN_DELAY 0.050000
#define MAX_RESPAWN_BEFORE_DELAY 5
#define MINIMUM_COUNT_VALUE 1

extern pid_t child;
extern struct rlimit owner_limits[RLIM_NLIMITS];

/**
 * @fn int watchdog_spawn(char* name, char** args, char** envs)
 * @brief Start a process and wait for the exit.
 * @param name The process name.
 * @param args The process args.
 * @param envs The process envs.
 * @return -1 on error else 0.
 */
int watchdog_spawn(char* name, char** args, char** envs) {
  int status, i;
  pid_t w;
  if((child = fork()) >= 0) {// fork was successful
    if(child == 0) {// child process
      /* set the owner limits */
      for(i = 0; i < RLIM_NLIMITS; i++)
	setrlimit(i, &owner_limits[i]);
      if(execve(name, args, envs) == -1) {
	fprintf(stderr, "Unable to starts the process name:'%s', path: '%s': (%d) %s.\n", args[0], name, errno, strerror(errno));
      }
      _exit(0);
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
    return -1;
  return 0;
}

/**
 * @fn int watchdog_respawn(char* name, char** args, char** envs, _Bool disable_spam_detect)
 * @brief Start a process and restart if the process exit.
 * @param name The process name.
 * @param args The process args.
 * @param envs The process envs.
 * @param disable_spam_detect Disable the spam detection.
 * @return -1 on error else 0.
 */
int watchdog_respawn(char* name, char** args, char** envs, _Bool disable_spam_detect) {
  int result;
  struct timespec st, et;
  double elapsed = .0f;
  int count = MINIMUM_COUNT_VALUE;
  int delay;
  do {
    if(!disable_spam_detect) watchdog_utils_clock(&st);
    result = watchdog_spawn(name, args, envs);
    if(result) break;
    if(!disable_spam_detect) {
      watchdog_utils_clock(&et);
      elapsed = watchdog_utils_clock_elapsed(st, et);
      if(elapsed < MINIMUM_RESPAWN_DELAY) {
	if(count == MAX_RESPAWN_BEFORE_DELAY) {
	  fprintf(stderr, "Too many restarts for the process '%s'.\n", args[0]);
	  exit(EXIT_FAILURE);
	}
	count++;
	delay = count * (elapsed*MICRO_VALUE);
	usleep(delay);
      } else count = MINIMUM_COUNT_VALUE;
    }
  } while(1);
  return result;
}
