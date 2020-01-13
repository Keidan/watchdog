/**
 * @file watchdog_run.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#include "watchdog.hpp"
#include <sys/types.h>
#include <sys/wait.h>


constexpr float MINIMUM_RESPAWN_DELAY = 0.050000;
constexpr std::uint32_t MAX_RESPAWN_BEFORE_DELAY = 5;
constexpr std::uint32_t MINIMUM_COUNT_VALUE = 1;


WRun::WRun(rlimit_t &ownerLimits) : m_child(-1), m_ownerLimits(ownerLimits)
{
}

WRun::~WRun()
{
  if(m_child != -1) 
  {
    kill(m_child, SIGTERM);
    m_child = -1;
  }
}


/**
 * @brief Start a process and wait for the exit.
 * @param name The process name.
 * @param args The process args.
 * @param envs The process envs.
 * @return false on error else true.
 */
auto WRun::spawn(std::string &name, std::vector<std::string> &args, std::vector<std::string> &envs) -> bool
{
  int status;
  if((m_child = fork()) >= 0) /* fork success */
  {
    if(m_child == 0) /* child process */
    {
      /* set the owner limits */
      auto i = 0U;
      for(rlimit_t::iterator it = m_ownerLimits.begin(); it != m_ownerLimits.end(); ++it, i++)
        setrlimit(i, it);
      if(execve(name.c_str(), WUtils::toStrlist(args).data(), WUtils::toStrlist(envs).data()) == -1) 
      {
        std::cerr << "Unable to starts the process name:'" << args[0] << "', path: '" << name << "': (" << errno << ") " << strerror(errno) << "." << std::endl;
      }
      _exit(0);
    }
    else /* Parent process */
    {
      auto w = waitpid(m_child, &status, WUNTRACED | WCONTINUED);
      if (w == -1) {
        std::cerr << "Unable to wait for the process '" << args[0] << "[" << m_child << "]' : (" << errno << ") " << strerror(errno) << "." << std::endl;
        exit(EXIT_FAILURE);
      }
      m_child = 0;
    }
  } 
  else /* fork failed */
    return false;
    
  return true;
}

/**
 * @brief Start a process and restart if the process exit.
 * @param name The process name.
 * @param args The process args.
 * @param envs The process envs.
 * @param disableSpamDetect Disable the spam detection.
 * @return false on error else true.
 */
auto WRun::respawn(std::string &name, std::vector<std::string> &args, std::vector<std::string> &envs, bool disableSpamDetect) -> bool
{
  bool result;
  struct timespec st, et;
  auto count = MINIMUM_COUNT_VALUE;
  auto delay = 0UL;
  do 
  {
    if(!disableSpamDetect) 
      WUtils::clock(&st);
      
    result = spawn(name, args, envs);
    if(!result) break;
    
    if(!disableSpamDetect) 
    {
      WUtils::clock(&et);
      auto elapsed = WUtils::clockElapsed(st, et);
      if(elapsed < MINIMUM_RESPAWN_DELAY) 
      {
        if(count == MAX_RESPAWN_BEFORE_DELAY) 
        {
          std::cerr << "Too many restarts for the process '" << args[0] << "'." << std::endl;
          exit(EXIT_FAILURE);
        }
        count++;
        delay = count * (elapsed * MICRO_VALUE);
        usleep(delay);
      } 
      else 
        count = MINIMUM_COUNT_VALUE;
    }
  } while(true);
  
  return result;
}
