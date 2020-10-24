/**
 * @file watchdog.hpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#ifndef __WATCHDOG_HPP__
#define __WATCHDOG_HPP__

/* Includes -----------------------------------------------------------------*/
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <libgen.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <vector>
#include <array>
#include <sys/types.h>
#include <sys/stat.h>
#include "config.hpp"
#include "Log.hpp"

/* Public variables ---------------------------------------------------------*/
constexpr std::uint32_t MINIMUM_COUNT_VALUE = 1;
constexpr std::uint32_t MAX_RESPAWN = 5;
constexpr float MINIMUM_RESPAWN_DELAY = 0.050000;
constexpr float NANO_VALUE =  1000000000.0;
constexpr float MICRO_VALUE = 1000000.0;

/* Public macros ------------------------------------------------------------*/
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
#ifndef CONFIG_FILE_FOLDER
#define CONFIG_FILE_FOLDER "/etc/watchdog"
#endif
#endif /* HAVE_XML_H HAVE_JSON_H */

#ifndef PID_FOLDER
#define PID_FOLDER "/var/run"
#endif

class WProcess
{
  public:
    WProcess() = default;
    virtual ~WProcess() = default;
    
    std::string name;
    std::string path;
    std::string working;
    std::vector<std::string> args;
    std::vector<std::string> envs;
};

/* Public classes -----------------------------------------------------------*/
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
/****************
 * Loader interface
 ****************/
class WLoader
{
  public:
    WLoader(struct WProcess* process) : m_process(process)
    {
    }
    virtual ~WLoader() = default;
    
    /**
     * @brief Loads the file passed in parameter.
     * @param[in] filename The path of the file.
     * @retval true on error, otherwise false.
     */
    virtual auto load(const std::string &filename) -> bool = 0;

    /**
     * @brief Returns the file template.
     * @retval const char*
     */
    virtual auto getTemplate() -> const char* = 0;
  protected:
    struct WProcess* m_process;
};
#ifdef HAVE_JSON_H
/****************
 * JSON
 ****************/
class WJSON : public WLoader
{
  public:
    WJSON(struct WProcess* process);
    virtual ~WJSON() = default;
      
    /**
     * @brief Loads the file passed in parameter.
     * @param[in] filename The path of the file.
     * @retval true on error, otherwise false.
     */
    virtual auto load(const std::string &filename) -> bool;
    
    /**
     * @brief Returns the file template.
     * @retval const char*
     */
    virtual auto getTemplate() -> const char*;
};
#endif /* HAVE_JSON_H */
#ifdef HAVE_XML_H
/****************
 * XML
 ****************/
  
class WXML : public WLoader
{
  public:
    WXML(struct WProcess* process);
    virtual ~WXML() = default;
      
    /**
     * @brief Loads the file passed in parameter.
     * @param[in] filename The path of the file.
     * @retval true on error, otherwise false.
     */
    virtual auto load(const std::string &filename) -> bool;
    
    /**
     * @brief Returns the file template.
     * @retval const char*
     */
    virtual auto getTemplate() -> const char*;

};
#endif /* HAVE_XML_H */
#endif /* HAVE_XML_H HAVE_JSON_H */

/****************
 * UTILS
 ****************/
class WUtils
{
  private:
    WUtils() = default;
    virtual ~WUtils() = default;
       
  public:
    constexpr static mode_t DEFAULT_DIR_MODE = S_IRWXU | S_IRGRP |  S_IXGRP | S_IROTH | S_IXOTH;

     
    /**
     * @brief Creates the directory as well as the parents if they don't exist.
     * @param[in] s Le chemin à créer
     * @param[in] mode The directory mode.
     * @retval false on error, otherwise true.
     */
    static auto mkdirs(const std::string &s, mode_t mode = DEFAULT_DIR_MODE) -> bool;
      
    /**
     * @brief Converts a string vector to char**.
     * @param[in] input The vector of string.
     * @retval std::vector<char*>
     * Must be used as follows:
     * char** result = toStrlist(input).data();
     */
    static auto toStrlist(const std::vector<std::string> &input) -> std::vector<char*>;

    /**
     * @brief Gets the elapsed time in nanosec between 2 timespec.
     * @param[in] s Starting value.
     * @param[in] e Final value.
     * @retval The elapsed time.
     */
    static auto clockElapsed(const struct timespec &s, const struct timespec &e) -> double;

    /**
     * @brief Gets the current time using the monotonic clock.
     * @param[out] ts The time.
     * @retval false on error else true.
     */
    static auto clock(struct timespec *ts) -> bool;

    /**
     * @brief Searches for a file in the PATH system.
     * @param[in] name The name of the file.
     * @param[out] path The output path with the filename.
     * @retval false on error, otherwise true.
     */
    static auto findFile(const std::string &name, std::string &path) -> bool;

    /**
     * @brief Concatenates process environment variables with system environment variables.
     * @param[out] process The process context.
     */
    static auto completeEnv(struct WProcess* process) -> void;
    
  private:
    /**
     * @brief Splits a string according to the specified regex
     * @param[in] in The input string.
     * @param[in] reg The regex.
     * @retval The result in a vector.
     */
    static auto split(const std::string& in, const std::string &reg) -> std::vector<std::string>;
};

/****************
 * RUN
 ****************/
using rlimit_t = std::array<struct rlimit, RLIM_NLIMITS>;

class WRun
{
  public:
    WRun(rlimit_t &ownerLimits, std::string &working);
    virtual ~WRun();
      
    /**
     * @brief Starts a process and waits for output.
     * @param[in] name The name of the process.
     * @param[in] args The args process.
     * @param[in] envs The envs process.
     * @retval false on error, otherwise true.
     */
    auto spawn(const std::string &name, const std::vector<std::string> &args, const std::vector<std::string> &envs) -> bool;
    
    /**
     * @brief Starts a process and restarts it if the process ends.
     * @param[in] name The name of the process.
     * @param[in] args The args process.
     * @param[in] envs The envs process.
     * @param[in] disableSpamDetect Disables spam detection.
     * @param[in] maxRespawn The maximum number of respawn allowed.
     * @param[in] minRespawnDelay The minimum respawn delay before starting spam detection.
     * @retval false on error, otherwise true.
     */
    auto respawn(const std::string &name, const std::vector<std::string> &args, const std::vector<std::string> &envs, bool disableSpamDetect, std::uint32_t maxRespawn, float minRespawnDelay) -> bool;
      
  private:
    pid_t m_child;
    rlimit_t &m_ownerLimits;
    std::string &m_working;
};

#endif /* __WATCHDOG_HPP__ */
