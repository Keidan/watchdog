/**
 * @file Watchdog.hpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#pragma once

/* Includes -----------------------------------------------------------------*/
#include <wd/BuildConfig.hpp>
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
#  include <wd/config/ConfigJSON.hpp>
#  include <wd/config/ConfigXML.hpp>
#else
#  include <wd/config/IConfigable.hpp>
#endif /* defined(HAVE_XML_H) || defined(HAVE_JSON_H) */
#include <wd/run/Process.hpp>

/* Class --------------------------------------------------------------------*/
namespace wd
{
  enum class ConfigType : std::uint8_t
  {
    NONE = 0,
    XML,
    JSON
  };

  constexpr std::uint32_t MAX_RESPAWN = 5;
  constexpr std::int64_t MINIMUM_RESPAWN_DELAY_MS = 500;

  class WatchdogConfig
  {
    public:
      WatchdogConfig() = default;
      ~WatchdogConfig() = default;

      std::string binaryName{};
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
      std::string configName{};
      bool createNew = false;
#endif /* defined(HAVE_XML_H) || defined(HAVE_JSON_H) */
      bool disableSpamDetect = false;
      std::uint32_t maxRespawn = MAX_RESPAWN;
      std::int64_t minRespawnDelay = MINIMUM_RESPAWN_DELAY_MS;
  };

  class PID
  {
    public:
      PID() = default;
      ~PID() = default;

      std::string folder{};
      std::string file{};
      pid_t pid = -1;
  };

  class Watchdog
  {
    public:
      Watchdog() = default;
      ~Watchdog() = default;

      /**
       * @brief Loads the internal contexts.
       * 
       * @param[in] configType The config type.
       * @param[in] process The associated process.
       */
      auto load([[maybe_unused]] ConfigType configType, [[maybe_unused]] std::shared_ptr<run::Process> process) -> void;

      /**
       * @brief Kills the process and deletes the pid file.
       */
      auto kill() const -> void;

      /**
       * @brief Defines the PID configuration to be used.
       *
       * @param[in] cfgPid The config.
       * @param[in] bname The base name of the process.
       */
      auto setConfigPID(std::shared_ptr<PID> cfgPid, std::string_view bname) -> void;

      /**
       * @brief Executes the process.
       *
       * @param[in] wdConfig Configuration of the watchdog (corresponds to the direct arguments of the application.
       * @param[in] process The associated process.
       * @retval false on error.
       */
      auto execute(const WatchdogConfig& wdConfig, const std::shared_ptr<run::Process>& process) const -> bool;

    private:
      std::string m_pfile{};
      std::shared_ptr<PID> m_cfgPid = nullptr;
      std::shared_ptr<config::IConfigable> m_config = nullptr;

#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
      /**
       * @brief Creates the configuration file.
       *
       * @param[in] filename The configuration file name.
       * @retval false on error.
       */
      auto createConfig(std::string_view filename) const -> bool;

#endif /* HAVE_XML_H HAVE_JSON_H */

      /**
       * @brief Wrtie the current PID.
       *
       * @retval false on error.
       */
      auto writePID() const -> bool;
  };
} // namespace wd