/**
 * @file Helper.hpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#pragma once

/* Includes -----------------------------------------------------------------*/
#include <array>
#include <chrono>
#include <errno.h>
#include <filesystem>
#include <memory>
#include <regex>
#include <string_view>
#include <sys/resource.h>
#include <unistd.h>
#include <wd/run/Process.hpp>
/* Exported type ------------------------------------------------------------*/

/* Class --------------------------------------------------------------------*/
namespace wd::utils
{
  using rlimit_t = std::array<struct rlimit, RLIM_NLIMITS>;
  /****************
   * Helper
   ****************/
  class Helper
  {
    private:
      Helper() = default;
      virtual ~Helper() = default;

    public:
      /**
       * @brief Get the soft and hard limits for RESOURCE to *RLIMITS.
       *
       * @retval The limits.
       */
      static auto getLimits() -> rlimit_t
      {
        rlimit_t limits;
        auto i = 0U;
        for (auto& limit : limits)
        {
          getrlimit(i, &limit);
          i++;
        }
        return limits;
      }

      /**
       * @brief Set the soft and hard limits for RESOURCE to *RLIMITS.
       *
       * @param[in] limits The limits.
       */
      static auto setLimits(const rlimit_t& limits) -> void
      {
        /* set the owner limits */
        auto i = 0U;
        for (const auto& limit : limits)
        {
          setrlimit(i, &limit);
          i++;
        }
      }
      /**
       * @brief Creates the directory as well as the parents if they don't exist.
       *
       * @param[in] s The path to be created.
       * @retval false on error, otherwise true.
       */
      static auto mkdirs(const std::string& s) -> bool
      {
        std::filesystem::path dir = s;
        auto ret = true;
        const auto perms = std::filesystem::perms::owner_all | std::filesystem::perms::group_read | std::filesystem::perms::group_exec |
                           std::filesystem::perms::others_read | std::filesystem::perms::others_exec;
        if (!std::filesystem::exists(dir))
        {
          try
          {
            ret = std::filesystem::create_directories(dir);
          }
          catch (const std::filesystem::filesystem_error& ex)
          {
            (void)ex;
            ret = false;
          }
        }
        if (ret && perms != std::filesystem::status(dir).permissions())
          std::filesystem::permissions(dir, perms, std::filesystem::perm_options::replace);
        return ret;
      }

      /**
       * @brief Gets the elapsed time in nanosec between 2 timespec.
       *
       * @param[in] start Starting value.
       * @retval The elapsed time.
       */
      template <class clock_t = std::chrono::steady_clock, class duration_t = std::chrono::milliseconds>
      static auto clockElapsed(std::chrono::time_point<clock_t, duration_t> const& start) -> std::int64_t
      {
        return std::chrono::duration_cast<std::chrono::milliseconds>(clock_t::now() - start).count();
      }

      /**
       * @brief Converts a string vector to a const char* vector.
       *
       * @param[in] origin The original vector.
       * @param[in] sentienl Adds the sentinel ?
       * @retval The output vector.
       */
      static auto toCArray(const std::vector<std::string>& origin, bool sentinel = true) -> std::vector<const char*>
      {
        std::vector<const char*> output;
        output.reserve(origin.size());
        for (const auto& str : origin)
        {
          output.emplace_back(str.c_str());
        }
        if (sentinel)
          output.emplace_back(nullptr);
        return output;
      }

      /**
       * @brief Searches for a file in the PATH system.
       *
       * @param[in] name The name of the file.
       * @param[out] path The output path with the filename.
       * @retval false on error, otherwise true.
       */
      static auto findFile(const std::string& name, std::string& path) -> bool
      {
        auto paths = getenv("PATH");
        path.clear();

        auto splits = split(paths, ":");
        for (const auto& str : splits)
        {
          path.append(str);

          normalizePath(path);
          auto full = path + name;

          if (!std::filesystem::exists(full))
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
       *
       * @param[out] process The process context.
       */
      static auto completeEnv(const std::shared_ptr<run::Process>& process) -> void
      {
        bool found;

        for (auto c_env = environ; *c_env; ++c_env)
        {
          auto env = *c_env;
          found = false;
          for (const auto& s_env : process->getEnvs())
          {
            if (auto idx = s_env.find("="); std::string::npos != idx && 0 == s_env.rfind(env, 0, idx))
            {
              found = true;
              break;
            }
          }
          if (!found)
          {
            process->getEnvs().emplace_back(env);
          }
        }
      }

      /**
       * @brief Normalizes the path.
       *
       * @param[in,out] path The path.
       */
      static auto normalizePath(std::string& path) -> void
      {
        if ('/' != path.back())
        {
          path.append("/");
        }
      }

      /**
       * @brief Changes the current dir.
       *
       * @param[in] path The new path.
       * @param[in] returnIfEmpty The value to return if the path is empty.
       * @retval false on error.
       */
      static auto changeDir(std::string_view path, bool returnIfEmpty = true) -> bool
      {
        auto ret = false;
        if (!path.empty())
        {
          /* Adjusts the current directory to the directory specified by the configuration. */
          if (std::filesystem::exists(path))
          {
            std::filesystem::current_path(path);
            if (std::filesystem::current_path() == path)
            {
              ret = true;
            }
          }
        }
        else
        {
          ret = returnIfEmpty;
        }
        return ret;
      }

    private:
      /**
       * @brief Splits a string according to the specified regex
       *
       * @param[in] in The input string.
       * @param[in] reg The regex.
       * @retval The result in a vector.
       */
      static auto split(std::string_view in, const std::string& reg) -> std::vector<std::string>
      {
        // passing -1 as the submatch index parameter performs splitting
        std::regex re(reg);
        std::string input(in);
        std::sregex_token_iterator first{input.begin(), input.end(), re, -1};
        std::sregex_token_iterator last;
        return {first, last};
      }
  };
} // namespace wd::utils