/**
 * @file Log.hpp (clog wrapper)
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#ifndef __LOG_HPP__
#define __LOG_HPP__

/* Includes -----------------------------------------------------------------*/
#include <streambuf>
#include <syslog.h>

/* Public defines -----------------------------------------------------------*/
enum class LogPriority : std::uint8_t
{
    EMERG   = LOG_EMERG,
    ALERT   = LOG_ALERT,
    CRIT    = LOG_CRIT,
    ERR     = LOG_ERR,
    WARNING = LOG_WARNING,
    NOTICE  = LOG_NOTICE,
    INFO    = LOG_INFO,
    DBG     = LOG_DEBUG,
};

std::ostream& operator<< (std::ostream& os, const LogPriority& priority);


/* Public classes -----------------------------------------------------------*/
class Log : public std::basic_streambuf<char, std::char_traits<char> >
{
  public:
    explicit Log(std::string ident, int facility);
    virtual ~Log();
    
  protected:
    int sync();
    int overflow(int c);

  private:
    friend std::ostream& operator<< (std::ostream& os, const LogPriority& priority);
    std::string m_buffer;
    LogPriority m_priority;
    char m_idt[50];
};

#endif /* __LOG_HPP__ */
