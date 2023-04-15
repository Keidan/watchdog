/**
 * @file Process.hpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#pragma once

/* Includes -----------------------------------------------------------------*/
#include <array>
#include <string>
#include <string_view>
#include <sys/resource.h>
#include <sys/time.h>
#include <vector>
#include <wd/utils/Helper.hpp>

/* Class --------------------------------------------------------------------*/
namespace wd::run
{

  /****************
   * RUN
   ****************/

  class Run
  {
    public:
      Run(wd::utils::rlimit_t& limits, std::string_view working);
      virtual ~Run();

      /**
       * @brief Starts a process and waits for output.
       *
       * @param[in] name The name of the process.
       * @param[in] args The args process.
       * @param[in] envs The envs process.
       * @retval false on error, otherwise true.
       */
      auto spawn(const std::string& name, const std::vector<std::string>& args, const std::vector<std::string>& envs) -> bool;

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
      auto respawn(const std::string& name, const std::vector<std::string>& args, const std::vector<std::string>& envs, bool disableSpamDetect,
                   std::uint32_t maxRespawn, std::int64_t minRespawnDelay) -> bool;

    private:
      pid_t m_child = -1;
      wd::utils::rlimit_t& m_limits;
      std::string m_working;
      std::vector<const char*> m_cArgs;
      std::vector<const char*> m_cEnvs;

      /**
       * @brief Elapsed time management.
       *
       * @param[in,out] count The number of respawn.
       * @param[in] ref Ref time.
       * @param[in] maxRespawn The maximum number of respawn allowed.
       * @param[in] minRespawnDelay The minimum respawn delay before starting spam detection.
       * @retval false on error.
       */
      template <class clock_t = std::chrono::steady_clock, class duration_t = std::chrono::milliseconds>
      auto processElapsed(std::uint32_t& count, std::chrono::time_point<clock_t, duration_t> const& ref, std::uint32_t maxRespawn,
                          std::int64_t minRespawnDelay) const -> bool;
  };
} // namespace wd::run