/**
 * @file Log.hpp (clog wrapper)
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#pragma once

/* Includes -----------------------------------------------------------------*/
#include <streambuf>
#include <string_view>
#include <syslog.h>

/* Exported type ------------------------------------------------------------*/
namespace wd::utils
{
  enum class LogPriority : std::uint8_t
  {
    EMERG = LOG_EMERG,
    ALERT = LOG_ALERT,
    CRIT = LOG_CRIT,
    ERR = LOG_ERR,
    WARNING = LOG_WARNING,
    NOTICE = LOG_NOTICE,
    INFO = LOG_INFO,
    DBG = LOG_DEBUG,
  };
} // namespace wd::utils

std::ostream& operator<<(std::ostream& os, const wd::utils::LogPriority& priority);

/* Class --------------------------------------------------------------------*/
namespace wd::utils
{
  class Log : public std::basic_streambuf<char, std::char_traits<char>>
  {
    public:
      explicit Log(std::string_view ident, int facility);
      ~Log() override;
      Log(Log const&) = delete;
      Log& operator=(Log const&) = delete;

      auto setPriority(const LogPriority& priority) -> void;

    protected:
      auto sync() -> int override;
      auto overflow(int c) -> int override;

    private:
      std::string m_buffer{};
      LogPriority m_priority = LogPriority::INFO;
      std::string m_idt{};
  };
} // namespace wd::utils
