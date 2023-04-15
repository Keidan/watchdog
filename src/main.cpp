/**
 * @file main.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
/* Includes -----------------------------------------------------------------*/
#include <cstring>
#include <filesystem>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <signal.h>
#include <wd/Watchdog.hpp>
#include <wd/utils/Helper.hpp>
#include <wd/utils/Log.hpp>

/* Usings -------------------------------------------------------------------*/
using wd::ConfigType;
using wd::PID;
using wd::Watchdog;
using wd::WatchdogConfig;
using wd::run::Process;
using wd::utils::Helper;
using wd::utils::Log;
using wd::utils::LogPriority;

/* Private define -----------------------------------------------------------*/
#ifndef CONFIG_FILE_FOLDER
static constexpr auto* CONFIG_FOLDER = CONFIG_FILE_FOLDER;
#else
static constexpr auto* CONFIG_FOLDER = "/etc/watchdog";
#endif /* CONFIG_FILE_FOLDER */
static constexpr auto* PID_FOLDER = "/var/run";

#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
#  if defined(HAVE_XML_H) && !defined(HAVE_JSON_H)
static constexpr auto* USAGE_FEXT = "xml";
static constexpr auto* USAGE_FTYPE = "possible value: xml";
#  elif !defined(HAVE_XML_H) && defined(HAVE_JSON_H)
static constexpr auto* USAGE_FEXT = "json";
static constexpr auto* USAGE_FTYPE = "possible value: json";
#  elif defined(HAVE_XML_H) || defined(HAVE_JSON_H)
static constexpr auto* USAGE_FEXT = "[xml/json]";
static constexpr auto* USAGE_FTYPE = "possible value: xml or json";
#  endif /* HAVE_XML_H and/or HAVE_JSON_H */
#endif   /* defined(HAVE_XML_H) || defined(HAVE_JSON_H) */

/* Private variables --------------------------------------------------------*/
static const std::vector<struct option> long_options = {{"help", 0, nullptr, 'h'},        {"version", 0, nullptr, 'v'},
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
                                                        {"config", 1, nullptr, 'c'},      {"directory", 1, nullptr, 'd'},
                                                        {"new", 0, nullptr, 'z'},         {"type", 1, nullptr, 't'},
#endif /* HAVE_XML_H HAVE_JSON_H */
                                                        {"path", 1, nullptr, 'p'},        {"working", 1, nullptr, 'w'},
                                                        {"name", 1, nullptr, 'n'},        {"arg", 1, nullptr, 'a'},
                                                        {"env", 1, nullptr, 'e'},         {"pid", 0, nullptr, '0'},
                                                        {"pidfile", 1, nullptr, '1'},     {"disable-spam-detect", 0, nullptr, '2'},
                                                        {"max-respawn", 1, nullptr, '3'}, {"min-respawn-delay", 1, nullptr, '4'},
                                                        {nullptr, 0, nullptr, 0}};
static const std::unique_ptr<Watchdog> watchdog = std::make_unique<Watchdog>();
/* Static forward -----------------------------------------------------------*/
static auto getConfigType([[maybe_unused]] const char* input, [[maybe_unused]] bool fromExt) -> ConfigType;
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
static auto prepareConfigFile(WatchdogConfig& wdConfig, const ConfigType configType, std::string_view path) -> void;
#endif /* HAVE_XML_H HAVE_JSON_H */
static auto atExit(void) -> void;
[[noreturn]] static auto sigExit(int s) -> void;
[[noreturn]] static auto usage(std::string_view name, int err) -> void;

/* Public functions ---------------------------------------------------------*/
auto main(int argc, char** argv) -> int
{
  auto process = std::make_shared<Process>();
  WatchdogConfig wdConfig{};
  auto configPID = std::make_shared<PID>();

  auto configType = ConfigType::NONE;
  struct sigaction sa;

  std::string path = "";

  atexit(atExit);
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sigExit;
  (void)sigaction(SIGINT, &sa, nullptr);
  (void)sigaction(SIGTERM, &sa, nullptr);
  (void)sigaction(SIGTRAP, &sa, nullptr);

  configPID->folder = PID_FOLDER;
  wdConfig.binaryName = std::filesystem::path(argv[0]).filename();

  std::clog.rdbuf(new Log(wdConfig.binaryName, LOG_LOCAL0));

  /* parse the arguments */
  int opt;
  while ((opt = getopt_long(argc, argv, "hvc:zd:n:a:e:p:w:01:23:4:t:", &long_options[0], nullptr)) != -1)
  {
    switch (opt)
    {
    case 'h':
      usage(wdConfig.binaryName, 0);
      break;
    case 'v':
      std::cout << "Watchdog version " << VERSION_MAJOR << "." << VERSION_MINOR << std::endl;
      exit(0);
      break;
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
    case 'c': /* configuration file */
      wdConfig.configName = optarg;
      configType = getConfigType(wdConfig.configName.c_str(), true);
      break;
    case 'd': /* directory */
      path = optarg;
      break;
    case 't': /* file type */
      configType = getConfigType(optarg, false);
      break;
#endif        /* HAVE_XML_H HAVE_JSON_H */
    case '2': /* disable-spam-detect */
      wdConfig.disableSpamDetect = true;
      break;
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
    case 'z': /* new */
      wdConfig.createNew = true;
      break;
#endif        /* HAVE_XML_H HAVE_JSON_H */
    case '0': /* pid */
      configPID->pid = getpid();
      break;
    case '1': /* pidfile */
      configPID->file = optarg;
      configPID->pid = getpid();
      break;
    case 'p': /* path */
      process->setPath(optarg);
      break;
    case 'w': /* working */
      process->setWorking(optarg);
      break;
    case 'n': /* name */
      process->setName(optarg);
      process->getArgs().emplace_back(process->getName());
      break;
    case '3':
      try
      {
        wdConfig.maxRespawn = std::stoi(optarg);
      }
      catch (std::logic_error&)
      {
        std::clog << LogPriority::ERR << "Invalid max respawn value!" << std::endl;
        return EXIT_FAILURE;
      }
      break;
    case '4':
      try
      {
        wdConfig.minRespawnDelay = std::stoll(optarg);
      }
      catch (std::logic_error&)
      {
        std::clog << LogPriority::ERR << "Invalid min respawn delay value!" << std::endl;
        return EXIT_FAILURE;
      }
      break;
    case 'a': /* arg */
      process->getArgs().emplace_back(optarg);
      break;
    case 'e': /* env */
      process->getEnvs().emplace_back(optarg);
      break;
    default: /* '?' */
      usage(wdConfig.binaryName, EXIT_FAILURE);
      break;
    }
  }
  /* sanity check */
  if (wdConfig.minRespawnDelay < 0)
  {
    std::clog << LogPriority::EMERG << "Invalid min respawn delay value." << std::endl;
    usage(wdConfig.binaryName, EXIT_FAILURE);
  }

  watchdog->load(configType, process);
  watchdog->setConfigPID(configPID, wdConfig.binaryName);

#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)

  if (ConfigType::NONE != configType && process->getName().empty())
  {
    prepareConfigFile(wdConfig, configType, path);
  }
#endif /* HAVE_XML_H HAVE_JSON_H */
  auto r = watchdog->execute(wdConfig, process);
  watchdog->kill();
  /* Prevents the SIGTERM */
  exit(r ? EXIT_SUCCESS : EXIT_FAILURE);
  return EXIT_SUCCESS;
}

/* Static functions ---------------------------------------------------------*/
static auto getConfigType([[maybe_unused]] const char* input, [[maybe_unused]] bool fromExt) -> ConfigType
{
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
  if (fromExt)
  {
    auto p = std::filesystem::path(input);
    if (p.extension() == ".json")
      return ConfigType::JSON;
    else if (p.extension() == ".xml")
      return ConfigType::XML;
  }
  else
  {
    auto i = std::string(input);
    if (i == "json")
      return ConfigType::JSON;
    else if (i == "xml")
      return ConfigType::XML;
  }
#endif /* HAVE_XML_H HAVE_JSON_H */
  return ConfigType::NONE;
}
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
static auto prepareConfigFile(WatchdogConfig& wdConfig, const ConfigType configType, std::string_view path) -> void
{
  /* init the default filename */
  auto p = std::string(path);
  if (p.empty())
  {
    p = CONFIG_FOLDER;
  }
  Helper::normalizePath(p);
  if (!wdConfig.configName.empty())
  {
    auto temp = p;
    temp.append(wdConfig.configName);
    wdConfig.configName = temp;
  }
  else
  {
    wdConfig.configName = p;
    wdConfig.configName.append(wdConfig.binaryName);
    wdConfig.configName.append(".");
    if (ConfigType::JSON == configType)
      wdConfig.configName.append("json");
    else
      wdConfig.configName.append("xml");
  }
}
#endif /* HAVE_XML_H HAVE_JSON_H */

[[noreturn]] static auto usage(std::string_view name, int err) -> void
{
  std::cout << "Usage: watchdog options" << std::endl;
  std::cout << "\t--help, -h: Print this help." << std::endl;
  std::cout << "\t--version, -v: Print the version.." << std::endl;
  std::cout << "\t--disable-spam-detect: Disable spam detection." << std::endl;
  std::cout << "\t--pid: Write the pid in the file pointed by pidfile." << std::endl;
  std::cout << "\t--pidfile: The file containing the pid (default: " << PID_FOLDER << "/" << name << ".pid) (option enables pid support)" << std::endl;
  std::cout << "\t--max-respawn: The maximum number of respawn allowed (default: " << wd::MAX_RESPAWN << "; 0 for a start without respawn)" << std::endl;
  std::cout << "\t--min-respawn-delay: The minimum respawn delay before starting spam detection (default: " << wd::MINIMUM_RESPAWN_DELAY_MS << " miliseconds)"
            << std::endl;
  std::cout << "Stand-alone mode:" << std::endl;
  std::cout << "\t--path, -p: The process path (optional if the binary is in the PATH)." << std::endl;
  std::cout << "\t--working, -w: The working directory (optional)." << std::endl;
  std::cout << "\t--name, -n: The name of the process." << std::endl;
  std::cout << "\t--arg, -a: The process argument (repeat if there're several)." << std::endl;
  std::cout << "\t--env, -e: The process environment variable (repeat if there're several)." << std::endl;

#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
  std::cout << "File mode:" << std::endl;
  std::cout << "\t--directory, -d: The directory to find the configuration file (default: " << CONFIG_FOLDER << ")." << std::endl;
  std::cout << "\t--file-type, -t: The file type (" << USAGE_FTYPE << ")." << std::endl;
  std::cout << "\t--config, -c: Loads a configuration file." << std::endl;
  std::cout << "\t--new, -z: Creates a new configuration file." << std::endl;
  std::cout << "\tIf no configuration file is passed as parameter." << std::endl;
  std::cout << "\tIf no configuration file is passed as a parameter, the application looks for a configuration file located in the  " << CONFIG_FOLDER
            << " directory and named: <the_application_name>." << USAGE_FEXT << std::endl;
  std::cout << "\tIt's possible to create symbolic links with several configuration files:" << std::endl;
  std::cout << "\tln -s watchdog watchdog-foo" << std::endl;
  std::cout << "\twatchdog-foo --new" << std::endl;
  std::cout << "\tYou can now edit the configuration file" << CONFIG_FOLDER << "/watchdog-foo." << USAGE_FEXT << std::endl;
#endif /* defined(HAVE_XML_H) || defined(HAVE_JSON_H) */
  exit(err);
}

static auto atExit() -> void
{
  if (nullptr != watchdog)
    watchdog->kill();
}

[[noreturn]] static auto sigExit([[maybe_unused]] int s) -> void
{
  if (s != EXIT_FAILURE)
    s = 0;
  exit(s);
}
