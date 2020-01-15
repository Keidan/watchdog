/**
 * @file watchdog.hpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#ifndef __WATCHDOG_HPP__
  #define __WATCHDOG_HPP__

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
  
  #define NANO_VALUE  1000000000.0
  #define MICRO_VALUE 1000000.0

  #define EMPTY_CONFIG_FILE "\
<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<!-- The process name eg with /bin/ls name=\"ls\" -->\n\
<process name=\"foo\">\n\
  <!-- Optional, if the file is not in the PATH -->\n\
  <path></path>\n\
  <!-- Optional, application working directory -->\n\
  <working></working>\n\
  <args>\n\
    <!-- Optional, arguments list -->\n\
    <arg></arg>\n\
  </args>\n\
  <envs>\n\
    <!-- Optional, environment variables list -->\n\
    <env name=\"\" value=\"\"/>\n\
  </envs>\n\
</process>\n"

  #ifndef CONFIG_FILE_FOLDER
    #define CONFIG_FILE_FOLDER "/etc/watchdog"
  #endif

  #ifndef PID_FOLDER
    #define PID_FOLDER "/var/run"
  #endif

  /****************
   * XML
   ****************/

  struct watchdog_xml_s {
    std::string name;
    std::string path;
    std::string working;
    std::vector<std::string> args;
    std::vector<std::string> envs;
  };
  
  class WXML {
    public:
      WXML(struct watchdog_xml_s* process);
      virtual ~WXML();
      
      /**
       * @brief Loads the XML file passed in parameter.
       * @param filename The path of the xml file.
       * @return true on success false.
       */
      auto load(std::string &filename) -> bool;
      
      /**
       * @brief Release the resources allocated by the load function.
       */
      auto unload() -> void;
      
    private:
      struct watchdog_xml_s* m_process;
  };

  /****************
   * UTILS
   ****************/
   
   class WUtils {
     private:
       WUtils() = default;
       virtual ~WUtils() = default;
       
     public:
        constexpr static mode_t DEFAULT_DIR_MODE = S_IRWXU | S_IRGRP |  S_IXGRP | S_IROTH | S_IXOTH;

     
       /**
        * @brief Create the directory as well as the parents if they do not exist.
        * @param s The path to create.
        * @param mode The directory mode.
        * @return false on error else true.
        */
      static auto mkdirs(std::string &s, mode_t mode = DEFAULT_DIR_MODE) -> bool;
      
      /**
       * @brief Converts a vector of string to char**
       * @param input The vector of string.
       * @return std::vector<char*>
       * Should be used as follows:
       * char** result = toStrlist(input).data();
       */
      static auto toStrlist(std::vector<std::string> &input) -> std::vector<char*>;
      
      /**
       * @brief Get the elapsed time in nanosec between 2 timespec.
       * @param s Start value.
       * @param e End value.
       * @return The elapsed time.
       */
      static auto clockElapsed(struct timespec &s, struct timespec &e) -> double;

      /**
       * @brief Get the current time using the monotonic clock.
       * @param ts The time.
       * @return false on error else true.
       */
      static auto clock(struct timespec *ts) -> bool;

      /**
       * @brief Find a file into the system PATH.
       * @param name The filename.
       * @param path The output path with the filename.
       * @return true on success else false.
       */
      static auto findFile(std::string &name, std::string &path) -> bool;

      /**
       * @brief Concate the process envs with the system envs.
       * @param process The process context.
       */
      static auto completeEnv(struct watchdog_xml_s* process) -> void;
  };

  /****************
   * RUN
   ****************/
  using rlimit_t = std::array<struct rlimit, RLIM_NLIMITS>;
  class WRun {
    public:
      WRun(rlimit_t &ownerLimits, std::string &working);
      virtual ~WRun();
      
      /**
       * @brief Start a process and wait for the exit.
       * @param name The process name.
       * @param args The process args.
       * @param envs The process envs.
       * @return false on error else true.
       */
      auto spawn(std::string &name, std::vector<std::string> &args, std::vector<std::string> &envs) -> bool;
      
      /**
       * @brief Start a process and restart if the process exit.
       * @param name The process name.
       * @param args The process args.
       * @param envs The process envs.
       * @param disableSpamDetect Disable the spam detection.
       * @return false on error else true.
       */
      auto respawn(std::string &name, std::vector<std::string> &args, std::vector<std::string> &envs, bool disableSpamDetect) -> bool;
      
    private:
      pid_t m_child;
      rlimit_t &m_ownerLimits;
      std::string &m_working;
  };

#endif /* __WATCHDOG_HPP__ */
