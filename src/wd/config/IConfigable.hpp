/**
 * @file IConfigable.hpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#pragma once

/* Includes -----------------------------------------------------------------*/
#include <wd/BuildConfig.hpp>
#include <wd/run/Process.hpp>

namespace wd::config
{
  /****************
   * Configable interface
   ****************/
  class IConfigable
  {
    public:
      IConfigable() = default;
      virtual ~IConfigable() = default;

      /**
       * @brief Loads the file passed in parameter.
       *
       * @param[in] filename The path of the file.
       * @retval false on error, otherwise true.
       */
      virtual auto load(const std::string& filename) -> bool = 0;

      /**
       * @brief Returns the file template.
       *
       * @retval std::string
       */
      virtual auto getTemplate() -> std::string = 0;
  };
} // namespace wd::config