/**
 * @file Log.cpp (clog wrapper)
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#include <cstring>
#include <iostream>
#include <ostream>
#include <wd/utils/Log.hpp>

using namespace wd::utils;

Log::Log(std::string_view ident, int facility) : m_idt(ident)
{
  m_idt.append("\0");
  openlog(m_idt.c_str(), LOG_PID | LOG_PERROR, facility);
}

Log::~Log() { closelog(); }

auto Log::sync() -> int
{
  if (m_buffer.length())
  {
    syslog(static_cast<std::underlying_type_t<LogPriority>>(m_priority), "%s", m_buffer.c_str());
    m_buffer.erase();
    m_priority = LogPriority::DBG; // default to debug for each message
  }
  return 0;
}

auto Log::overflow(int c) -> int
{
  if (EOF != c)
  {
    m_buffer += static_cast<char>(c);
  }
  else
  {
    sync();
  }
  return c;
}

auto Log::setPriority(const LogPriority& priority) -> void { m_priority = priority; }

std::ostream& operator<<(std::ostream& os, const wd::utils::LogPriority& priority)
{
  static_cast<wd::utils::Log*>(os.rdbuf())->setPriority(priority);
  return os;
}
