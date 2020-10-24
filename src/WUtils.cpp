/**
 * @file watchdog_utils.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
/* Includes -----------------------------------------------------------------*/
#include "watchdog.hpp"
#include <regex>

/* Usings -------------------------------------------------------------------*/
using std::string;
using std::vector;
using std::regex;
using std::sregex_token_iterator;

/* Public variables ---------------------------------------------------------*/
extern char **environ;

/* Public functions ---------------------------------------------------------*/
/**
 * @brief Creates the directory as well as the parents if they don't exist.
 * @param[in] s Le chemin à créer
 * @param[in] mode The directory mode.
 * @retval false on error, otherwise true.
 */
auto WUtils::mkdirs(const string &s, mode_t mode) -> bool
{
  size_t index = 0;
  string dir;
  string work = s;

  if(work[work.size()-1] != '/')
  {
    /* force trailing '/' */
    work += '/';
  }

  while((index = work.find_first_of('/', index)) != string::npos)
  {
    dir = work.substr(0, index++);
    if(dir.empty())
    {
      continue;
    }
    else if(mkdir(dir.c_str(), mode) && errno != EEXIST)
    {
      return false;
    }
  }
  return true;
}

/**
 * @brief Converts a string vector to char**.
 * @param[in] input The vector of string.
 * @retval std::vector<char*>
 * Must be used as follows:
 * char** result = toStrlist(input).data();
 */
auto WUtils::toStrlist(const vector<string> &input) -> vector<char*> 
{
    vector<char*> result;
    result.reserve(input.size()+1);
    for(vector<string>::const_iterator i = input.begin(); i != input.end(); ++i)
    {
      result.push_back(const_cast<char*>((*i).c_str()));
    }
    result.push_back(nullptr);
    return result;
}

/**
 * @brief Gets the elapsed time in nanosec between 2 timespec.
 * @param[in] s Starting value.
 * @param[in] e Final value.
 * @retval The elapsed time.
 */
auto WUtils::clockElapsed(const struct timespec &s, const struct timespec &e) -> double 
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
 * @brief Searches for a file in the PATH system.
 * @param[in] name The name of the file.
 * @param[out] path The output path with the filename.
 * @retval false on error, otherwise true.
 */
auto WUtils::findFile(const string &name, string &path) -> bool 
{
  auto paths = getenv ("PATH");
  path.clear();

  auto splits = split(paths, ":");
  for(vector<string>::iterator it = splits.begin(); it != splits.end(); ++it)
  {
    path.append(*it);
    if(path[path.length() - 1] != '/')
    {
      path.append("/");
    }
    auto full = path + name;
    
    if(access(full.c_str(), F_OK) != 0) 
    {
      path.clear();
    }
    else
    {
      break;
    }
  }
  return !path.empty();
}

/**
 * @brief Concatenates process environment variables with system environment variables.
 * @param[out] process The process context.
 */
auto WUtils::completeEnv(struct WProcess* process) -> void
{
  bool found;
  
  for (auto c_env = environ; *c_env; ++c_env) 
  {
    auto env = *c_env;
    found = false;
    if(!process->envs.empty()) 
    {
      for(vector<string>::iterator it = process->envs.begin(); it != process->envs.end(); ++it)
      {
        auto val = *it;
        auto idx = val.find("=");
        if(idx != string::npos)
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

/* Private functions --------------------------------------------------------*/
/**
 * @brief Splits a string according to the specified regex
 * @param[in] in The input string.
 * @param[in] reg The regex.
 * @retval The result in a vector.
 */
auto WUtils::split(const string& in, const string &reg) -> vector<string>
{
  // passing -1 as the submatch index parameter performs splitting
  regex re(reg);
  sregex_token_iterator first{in.begin(), in.end(), re, -1}, last;
  return {first, last};
  
}
