/**
 * @file Watchdog.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
/* Includes -----------------------------------------------------------------*/
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <wd/Watchdog.hpp>
#include <wd/run/Run.hpp>
#include <wd/utils/Helper.hpp>
#include <wd/utils/Log.hpp>

/* Usings -------------------------------------------------------------------*/
using namespace wd;

using run::Run;
using utils::Helper;
using utils::LogPriority;

/* Public functions ---------------------------------------------------------*/

/**
 * @brief Loads the internal contexts.
 *
 * @param[in] configType The config type.
 * @param[in] process The associated process.
 * @retval false on error.
 */
auto Watchdog::load([[maybe_unused]] ConfigType configType, [[maybe_unused]] std::shared_ptr<run::Process> process) -> void
{
#ifdef HAVE_JSON_H
  if (ConfigType::JSON == configType)
  {
    m_config = std::make_shared<config::ConfigJSON>(process);
  }
#endif /* HAVE_JSON_H */
#ifdef HAVE_XML_H
  if (ConfigType::XML == configType)
  {
    m_config = std::make_shared<config::ConfigXML>(process);
  }
#endif /* HAVE_XML_H */
}

/**
 * @brief Executes the process.
 *
 * @param[in] wdConfig Configuration of the watchdog (corresponds to the direct arguments of the application.
 * @param[in] process The associated process.
 * @retval false on error.
 */
auto Watchdog::execute(const WatchdogConfig& wdConfig, const std::shared_ptr<run::Process>& process) const -> bool
{

  auto loadFromFile = false;
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
  if (wdConfig.createNew)
  {
    if (wdConfig.configName.empty())
    {
      std::clog << LogPriority::EMERG << "The name of the configuration file cannot be empty." << std::endl;
      return false;
    }
    return createConfig(wdConfig.configName);
  }

  /* load the config file */
  if (nullptr != m_config && !m_config->load(wdConfig.configName))
  {
    return false;
  }
  loadFromFile = true;
#endif /* defined(HAVE_XML_H) || defined(HAVE_JSON_H) */

  if (process->getName().empty())
  {
    std::clog << LogPriority::EMERG << "The name of the process to be executed is empty." << std::endl;
    return false;
  }
  /* prepare the path and find the binary */
  if (process->getPath().empty() && !Helper::findFile(process->getName(), process->getPath()))
  {
    std::clog << LogPriority::EMERG << "The BIN file '" << process->getName() << "' was not found." << std::endl;
    return false;
  }
  if (!writePID())
  {
    return false;
  }

  auto limits = Helper::getLimits();

  /* add the parent env variables */
  Helper::completeEnv(process);
  auto cmdPath = process->getPath();
  if (!loadFromFile && process->getArgs().empty())
  {
    std::clog << LogPriority::EMERG << "No given process" << std::endl;
    return false;
  }
  Helper::normalizePath(cmdPath);
  cmdPath.append(process->getArgs()[0]);
  Run r(limits, process->getWorking());
  return r.respawn(cmdPath, process->getArgs(), process->getEnvs(), wdConfig.disableSpamDetect, wdConfig.maxRespawn, wdConfig.minRespawnDelay);
}

/**
 * @brief Kills the process and deletes the pid file.
 */
auto Watchdog::kill() const -> void
{
  if (nullptr != m_cfgPid && m_cfgPid->pid > 0)
  {
    std::ifstream file;
    file.open(m_pfile);

    if (file.is_open())
    {
      pid_t p;
      file >> p;
      file.close();
      if (p == m_cfgPid->pid)
      {
        ::kill(m_cfgPid->pid, SIGTERM);
        std::filesystem::remove(m_pfile);
      }
      m_cfgPid->pid = -1;
    }
  }
}

/**
 * @brief Defines the PID configuration to be used.
 *
 * @param[in] cfgPid The config.
 * @param[in] bname The base name of the process.
 */
auto Watchdog::setConfigPID(std::shared_ptr<PID> cfgPid, std::string_view bname) -> void
{
  m_cfgPid = std::move(cfgPid);
  if (nullptr != m_cfgPid)
  {
    if (m_cfgPid->file.empty())
      m_pfile = std::string(m_cfgPid->folder).append("/").append(bname).append(".pid");
    else
      m_pfile = m_cfgPid->file;
  }
}

/**
 * @brief Wrtie the current PID.
 *
 * @retval false on error.
 */
auto Watchdog::writePID() const -> bool
{
  if (nullptr == m_cfgPid || m_cfgPid->folder.empty())
    return false;

  if (m_cfgPid->pid > 0)
  {
    std::fstream fs;
    fs.open(m_pfile, std::fstream::in);
    if (fs.is_open())
    {
      std::clog << LogPriority::EMERG << "The pid file '" << m_pfile << "' already exists!" << std::endl;
      fs.close();
      return false;
    }
    fs.open(m_pfile, std::fstream::out);
    if (fs.is_open())
    {
      fs << m_cfgPid->pid << std::endl;
      fs.close();
    }
    else
    {
      std::clog << LogPriority::EMERG << "Unable to create the pid file '" << m_pfile << "': (" << errno << ") " << strerror(errno) << std::endl;
      return false;
    }
  }
  return true;
}

#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
/**
 * @brief Creates the configuration file.
 *
 * @param[in] filename The configuration file name.
 * @retval false on error.
 */
auto Watchdog::createConfig(std::string_view filename) const -> bool
{
  std::filesystem::path path = filename;

  if (auto dir = path.parent_path(); !Helper::mkdirs(dir))
  {
    std::clog << LogPriority::EMERG << "Unable to create directory '" << dir << "': (" << errno << ") " << strerror(errno) << std::endl;
    return false;
  }

  if (std::filesystem::exists(path) && !std::filesystem::remove(path))
    return false;

  std::ofstream outfile(path);
  if (!outfile || !outfile.is_open() || outfile.fail())
  {
    std::clog << LogPriority::EMERG << "Unable to create the file '" << filename << "': (" << errno << ") " << strerror(errno) << std::endl;
    return false;
  }
  auto s = m_config == nullptr ? std::string("") : m_config->getTemplate();
  if (s.empty())
  {
    return false;
  }
  outfile << s;
  outfile.close();
  return true;
}
#endif /* HAVE_XML_H HAVE_JSON_H */
