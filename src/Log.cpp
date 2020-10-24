/**
 * @file Log.cpp (clog wrapper)
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#include "Log.hpp"
#include <cstring>
#include <ostream>
#include <iostream>
Log::Log(std::string ident, int facility) : m_buffer(""), m_priority(LogPriority::DBG)
{
  strncpy(m_idt, ident.c_str(), sizeof(m_idt));
  m_idt[sizeof(m_idt)-1] = '\0';
  openlog(m_idt, LOG_PID|LOG_PERROR, facility);
}

Log::~Log()
{
  closelog();
}

int Log::sync()
{
  if (m_buffer.length())
  {
    syslog(static_cast<std::underlying_type<LogPriority>::type>(m_priority), "%s", m_buffer.c_str());
    m_buffer.erase();
    m_priority = LogPriority::DBG; // default to debug for each message
  }
  return 0;
}

int Log::overflow(int c)
{
  if (c != EOF)
  {
    m_buffer += static_cast<char>(c);
  }
  else
  {
    sync();
  }
  return c;
}

std::ostream& operator<< (std::ostream& os, const LogPriority& priority)
{
  static_cast<Log *>(os.rdbuf())->m_priority = priority;
  return os;
}
