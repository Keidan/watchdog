/**
 * @file Process.hpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#pragma once

/* Includes -----------------------------------------------------------------*/
#include <string>
#include <vector>

/* Class --------------------------------------------------------------------*/
namespace wd::run
{
  /****************
   * Process Model
   ****************/

  class Process
  {
    public:
      Process() = default;
      virtual ~Process() = default;

      auto setName(std::string name) -> void { m_name = std::move(name); }
      auto getName() const -> const std::string& { return m_name; }

      auto setPath(std::string path) -> void { m_path = std::move(path); }
      auto getPath() const -> const std::string& { return m_path; }
      auto getPath() -> std::string& { return m_path; }

      auto setWorking(std::string working) -> void { m_working = std::move(working); }
      auto getWorking() const -> const std::string& { return m_working; }

      auto getArgs() -> std::vector<std::string>& { return m_args; }
      auto getArgs() const -> const std::vector<std::string>& { return m_args; }

      auto getEnvs() -> std::vector<std::string>& { return m_envs; }
      auto getEnvs() const -> const std::vector<std::string>& { return m_envs; }

    private:
      std::string m_name{};
      std::string m_path{};
      std::string m_working{};
      std::vector<std::string> m_args{};
      std::vector<std::string> m_envs{};
  };
} // namespace wd