/**
 * @file watchdog_run.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
/* Includes -----------------------------------------------------------------*/
/* ugly hack part 1 */
#define execve xexecve
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <wd/run/Run.hpp>
#include <wd/utils/Log.hpp>

/*
 * ugly hack part 2 :
 * I agree this hack is really horrible but between C++ errors
 * and sonarlint that doesn't want dynamic allocations nor casts...
 * this is the only solution I found
 */
#undef execve
extern "C"
{
  int execve(const char* filename, const char** argvs, const char** envp);
#ifdef GCOV
  extern void __gcov_dump();
#endif /* GCOV */
}
/* Usings -------------------------------------------------------------------*/
using namespace wd::run;
using wd::utils::Helper;
using wd::utils::LogPriority;
using wd::utils::rlimit_t;


/* Public functions ---------------------------------------------------------*/
Run::Run(rlimit_t& limits, std::string_view working) : m_limits(limits), m_working(working) {}

/**
 * @brief Kills the child process.
 */
auto Run::killChild() -> void
{
  if (m_child != -1)
  {
    kill(m_child, SIGTERM);
    m_child = -1;
  }
}

/**
 * @brief Starts a process and waits for output.
 *
 * @param[in] name The name of the process.
 * @param[in] args The args process.
 * @param[in] envs The envs process.
 * @retval false on error, otherwise true.
 */
auto Run::spawn(const std::string& name, const std::vector<std::string>& args, const std::vector<std::string>& envs) -> bool
{
  int status;
  if (m_cArgs.empty())
    m_cArgs = Helper::toCArray(args);
  if (m_cEnvs.empty())
    m_cEnvs = Helper::toCArray(envs);
  if ((m_child = fork()) >= 0) /* fork success */
  {
    if (m_child == 0) /* child process */
    {
      Helper::setLimits(m_limits);
      if (!Helper::changeDir(m_working))
        std::clog << LogPriority::WARNING << "Unable to change the current directory:'" << m_working << "': (" << errno << ") " << strerror(errno) << "."
                  << std::endl;
#ifdef GCOV
      /* Force dump here */
      __gcov_dump();
      execve(name.c_str(), &m_cArgs[0], &m_cEnvs[0]);
#else
      if (-1 == execve(name.c_str(), &m_cArgs[0], &m_cEnvs[0]))
      {
        std::clog << LogPriority::EMERG << "Unable to starts the process name:'" << args[0] << "', path: '" << name << "': (" << errno << ") "
                  << strerror(errno) << "." << std::endl;
      }
#endif /* GCOV */
      _exit(0);
    }
    else /* Parent process */
    {
#ifdef GCOV
      waitpid(m_child, &status, WUNTRACED | WCONTINUED);
#else
      if (-1 == waitpid(m_child, &status, WUNTRACED | WCONTINUED))
      {
        std::clog << LogPriority::EMERG << "Unable to wait for the process '" << args[0] << "[" << m_child << "]' : (" << errno << ") " << strerror(errno)
                  << "." << std::endl;
      }
#endif /* GCOV */
      killChild();
      m_child = -1;
    }
  }
#ifndef GCOV
  else /* fork failed */
  {
    std::clog << LogPriority::EMERG << "Fork failed '" << args[0] << "' : (" << errno << ") " << strerror(errno) << "." << std::endl;
    return false;
  }
#endif /* GCOV */
  return true;
}

/**
 * @brief Starts a process and restarts it if the process ends.
 *
 * @param[in] name The name of the process.
 * @param[in] args The args process.
 * @param[in] envs The envs process.
 * @param[in] disableSpamDetect Disables spam detection.
 * @param[in] maxRespawn The maximum number of respawn allowed.
 * @param[in] minRespawnDelay The minimum respawn delay before starting spam detection.
 * @retval false on error, otherwise true.
 */
auto Run::respawn(const std::string& name, const std::vector<std::string>& args, const std::vector<std::string>& envs, bool disableSpamDetect,
                  std::uint32_t maxRespawn, std::int64_t minRespawnDelay) -> bool
{
  bool result;
  auto start = std::chrono::steady_clock::now();
  auto count = MINIMUM_COUNT_VALUE;
  do
  {
    if (!disableSpamDetect)
    {
      start = std::chrono::steady_clock::now();
    }
    result = spawn(name, args, envs);
    if (!result)
    {
      break;
    }
    else if (maxRespawn == 0)
    {
      return true;
    }
    if (!disableSpamDetect && !processElapsed(count, start, maxRespawn, minRespawnDelay))
    {
      std::clog << LogPriority::EMERG << "Too many restarts for the process '" << args[0] << "'." << std::endl;
      return false;
    }
  } while (true);

  return result;
}
