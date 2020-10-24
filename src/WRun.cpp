/**
 * @file watchdog_run.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
/* Includes -----------------------------------------------------------------*/
#include "watchdog.hpp"
#include <sys/types.h>
#include <sys/wait.h>


/* Usings -------------------------------------------------------------------*/
using std::string;
using std::vector;
using std::clog;
using std::endl;
using std::uint32_t;


/* Public functions ---------------------------------------------------------*/
WRun::WRun(rlimit_t &ownerLimits, string &working) :
  m_child(-1), m_ownerLimits(ownerLimits), m_working(working)
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
 * @brief Starts a process and waits for output.
 * @param[in] name The name of the process.
 * @param[in] args The args process.
 * @param[in] envs The envs process.
 * @retval false on error, otherwise true.
 */
auto WRun::spawn(const string &name, const vector<string> &args, const vector<string> &envs) -> bool
{
  int status;
  if((m_child = fork()) >= 0) /* fork success */
  {
    if(m_child == 0) /* child process */
    {
      /* set the owner limits */
      auto i = 0U;
      for(rlimit_t::iterator it = m_ownerLimits.begin(); it != m_ownerLimits.end(); ++it, i++)
      {
        setrlimit(i, it);
      }
      if(!m_working.empty())
      {
        /* Adjusts the current directory to the directory specified by the configuration. */
        if(chdir(m_working.c_str()) == -1)
	{
	  clog << LogPriority::ERR << "Unable to change the current directory:'" << m_working << "': (" << errno << ") " << strerror(errno) << "." << endl;
	}
      }
      if(execve(name.c_str(), WUtils::toStrlist(args).data(), WUtils::toStrlist(envs).data()) == -1) 
      {
        clog << LogPriority::ERR << "Unable to starts the process name:'" << args[0] << "', path: '" << name << "': (" << errno << ") " << strerror(errno) << "." << endl;
      }
      _exit(0);
    }
    else /* Parent process */
    {
      auto w = waitpid(m_child, &status, WUNTRACED | WCONTINUED);
      if (w == -1)
      {
        clog << LogPriority::EMERG << "Unable to wait for the process '" << args[0] << "[" << m_child << "]' : (" << errno << ") " << strerror(errno) << "." << endl;
        exit(EXIT_FAILURE);
      }
      m_child = 0;
    }
  } 
  else /* fork failed */
  {
    return false;
  }  
  return true;
}

/**
 * @brief Starts a process and restarts it if the process ends.
 * @param[in] name The name of the process.
 * @param[in] args The args process.
 * @param[in] envs The envs process.
 * @param[in] disableSpamDetect Disables spam detection.
 * @param[in] maxRespawn The maximum number of respawn allowed.
 * @param[in] minRespawnDelay The minimum respawn delay before starting spam detection.
 * @retval false on error, otherwise true.
 */
auto WRun::respawn(const std::string &name, const vector<string> &args, const vector<string> &envs, bool disableSpamDetect, uint32_t maxRespawn, float minRespawnDelay) -> bool
{
  bool result;
  struct timespec st, et;
  auto count = MINIMUM_COUNT_VALUE;
  auto delay = 0UL;
  do 
  {
    if(!disableSpamDetect)
    {
      WUtils::clock(&st);
    }  
    result = spawn(name, args, envs);
    if(!result)
    {
      break;
    }
    else if(maxRespawn == 0)
    {
      return true;
    }
    if(!disableSpamDetect) 
    {
      WUtils::clock(&et);
      auto elapsed = WUtils::clockElapsed(st, et);
      if(elapsed < minRespawnDelay) 
      {
        if(count == maxRespawn) 
        {
	  clog << LogPriority::EMERG << "Too many restarts for the process '" << args[0] << "'." << endl;
	  exit(EXIT_FAILURE);
        }
        count++;
        delay = count * (elapsed * MICRO_VALUE);
        usleep(delay);
      } 
      else 
        count = MINIMUM_COUNT_VALUE;
    }
  }
  while(true);
  
  return result;
}
