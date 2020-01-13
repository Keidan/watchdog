/**
 * @file watchdog_utils.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#include "watchdog.hpp"

extern char **environ;

/**
 * @brief Converts a vector of string to char**
 * @param input The vector of string.
 * @return std::vector<char*>
 * Should be used as follows:
 * char** result = toStrlist(input).data();
 */
auto WUtils::toStrlist(std::vector<std::string> &input) -> std::vector<char*> 
{
    std::vector<char*> result;
    result.reserve(input.size()+1);
    for(std::vector<std::string>::iterator i = input.begin(); i != input.end(); ++i)
        result.push_back(const_cast<char*>((*i).c_str()));
    result.push_back(nullptr);
    return result;
}

/**
 * @brief Get the elapsed time in nanosec between 2 timespec.
 * @param s Start value.
 * @param e End value.
 * @return The elapsed time.
 */
auto WUtils::clockElapsed(struct timespec &s, struct timespec &e) -> double 
{
  double elapsed = (double)(e.tv_sec - s.tv_sec);
  elapsed += (double)(e.tv_nsec - s.tv_nsec) / NANO_VALUE;
  return elapsed;
}

/**
 * @brief Get the current time using the monotonic clock.
 * @param ts The time.
 * @return false on error else true.
 */
auto WUtils::clock(struct timespec *ts) -> bool
{
  return clock_gettime(CLOCK_MONOTONIC, ts) != -1;
}

/**
 * @brief Find a file into the system PATH.
 * @param name The filename.
 * @param path The output path with the filename.
 * @return true on success else false.
 */
auto WUtils::findFile(std::string &name, std::string &path) -> bool 
{
  
  auto paths = getenv ("PATH");
  char* works, *tmp, *copy;
  path.clear();
  works = strdup(paths);
  copy = works;
  
  while((tmp = strsep(&works, ":")) != nullptr) {
    path.append(tmp);
    if(path[path.length() - 1] != '/')
      path.append("/");
    path.append(name);
    
    if(access(path.c_str(), F_OK) != 0) 
    {
      path.clear();
    }
    else
    {
      break;
    }
  }
  free(copy);
  return !path.empty();
}

/**
 * @brief Concate the process envs with the system envs.
 * @param process The process context.
 */
auto WUtils::completeEnv(struct watchdog_xml_s* process) -> void
{
  bool found;
  char **c_env, *env;
  
  for (c_env = environ; *c_env; ++c_env) 
  {
    env = *c_env;
    found = false;
    if(!process->envs.empty()) 
    {
      for(std::vector<std::string>::iterator it = process->envs.begin(); it != process->envs.end(); ++it)
      {
        auto val = *it;
        auto idx = val.find("=");
        if(idx != std::string::npos)
        {
          if (val.rfind(env, 0, idx) == 0) 
          {
              found = true;
              break;
          }
        }
      }
    }
    if(!found) 
    {
      process->envs.push_back(env);
    }
  }
}
