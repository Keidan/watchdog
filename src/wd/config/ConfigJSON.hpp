/**
 * @file ConfigJSON.hpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#pragma once

/* Includes -----------------------------------------------------------------*/
#include <memory>
#include <wd/BuildConfig.hpp>
#include <wd/config/IConfigable.hpp>
#include <wd/run/Process.hpp>

/* Class --------------------------------------------------------------------*/
#ifdef HAVE_JSON_H
namespace wd::config
{
  /****************
   * JSON
   ****************/
  class ConfigJSON : public IConfigable
  {
    public:
      explicit ConfigJSON(std::shared_ptr<run::Process> process);
      ~ConfigJSON() override = default;

      /**
       * @brief Loads the file passed in parameter.
       *
       * @param[in] filename The path of the file.
       * @retval false on error, otherwise true.
       */
      auto load(const std::string& filename) -> bool override;

      /**
       * @brief Returns the file template.
       *
       * @retval std::string
       */
      auto getTemplate() -> std::string override;

    private:
      std::shared_ptr<run::Process> m_process = nullptr;

      auto addNameValue(const std::string& name, const std::string& value) const -> void;
  };
} // namespace wd::config
#endif /* HAVE_JSON_H */