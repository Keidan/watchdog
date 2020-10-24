/**
 * @file watchdog.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
/* Includes -----------------------------------------------------------------*/
#include "watchdog.hpp"
#include <signal.h>
#include <getopt.h>
#include <fstream>

/* Usings -------------------------------------------------------------------*/
using std::ifstream;
using std::fstream;
using std::uint32_t;
using std::string;

/* Private define -----------------------------------------------------------*/
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
#define XML_TYPE "xml"
#define JSON_TYPE "json"
#endif /* HAVE_XML_H HAVE_JSON_H */

/* Private variables --------------------------------------------------------*/
static string pidfile = "";
static pid_t pid = -1;
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
static WLoader *wloader = nullptr;
#endif /* HAVE_XML_H HAVE_JSON_H */

static const struct option long_options[] = { 
  { "help"                   , 0, NULL, 'h' },
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
  { "config"                 , 1, NULL, 'c' },
  { "directory"              , 1, NULL, 'd' },
  { "new"                    , 0, NULL, 'z' },
  { "type"                   , 1, NULL, 't' },
#endif /* HAVE_XML_H HAVE_JSON_H */
  { "path"                   , 1, NULL, 'p' },
  { "working"                , 1, NULL, 'w' },
  { "name"                   , 1, NULL, 'n' },
  { "arg"                    , 1, NULL, 'a' },
  { "env"                    , 1, NULL, 'e' },
  { "pid"                    , 0, NULL, '0' },
  { "pidfile"                , 1, NULL, '1' },
  { "disable-spam-detect"    , 0, NULL, '2' },
  { "max-respawn"            , 1, NULL, '3' },
  { "min-respawn-delay"      , 1, NULL, '4' },
  { NULL                     , 0, NULL, 0   } 
};

/* Static forward -----------------------------------------------------------*/
static auto m_exit(void) -> void;
static auto sig_int(int s) -> void;
static auto usage(const string &name, int err) -> void;
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
static auto create(const string &filename) -> int;
#endif /* HAVE_XML_H HAVE_JSON_H */
static auto writePID(const string &bname, string &pidfile, pid_t pid) -> void;

/* Public functions ---------------------------------------------------------*/
auto main(int argc, char** argv) -> int
{
  string filename = "";
  struct sigaction sa;
  WProcess process;
  auto loadFromFile = false;
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
  auto createNew = false;
  string fileType = "";
#endif /* HAVE_XML_H HAVE_JSON_H */
  string path = "";
  string cmdPath = "";
  auto disableSpamDetect = false;
  auto maxRespawn = MAX_RESPAWN;
  auto minRespawnDelay = MINIMUM_RESPAWN_DELAY;
  
  atexit(m_exit);
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sig_int;
  (void)sigaction(SIGINT, &sa, NULL);
  (void)sigaction(SIGTERM, &sa, NULL);


  auto bname = std::string(basename(argv[0]));

  std::clog.rdbuf(new Log(bname, LOG_LOCAL0));
  
  process.name = "";
  process.path = "";
  
  /* parse the arguments */
  int opt;
  while ((opt = getopt_long(argc, argv, "hc:zd:n:a:e:p:w:01:23:4:t:", long_options, nullptr)) != -1) 
  {
    switch (opt) 
    {
      case 'h': 
        usage(bname, 0); 
        break;
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
      case 'c': /* configuration file */
        filename = string(optarg);
        break;
      case 'd': /* directory */
        path = string(optarg);
        break;
      case 't': /* file type */
        fileType = string(optarg);
        break;
#endif /* HAVE_XML_H HAVE_JSON_H */
      case '2': /* disable-spam-detect */
        disableSpamDetect = true;
        break;
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
      case 'z': /* new */
        createNew = true;
        break;
#endif /* HAVE_XML_H HAVE_JSON_H */
      case '0': /* pid */
        pid = getpid();
        break;
      case '1': /* pidfile */
        pidfile = string(optarg);
        pid = getpid();
        break;
      case 'p': /* path */
        process.path = string(optarg);
        break;
      case 'w': /* working */
        process.working = string(optarg);
        break;
      case 'n': /* name */
        process.name = string(optarg);
        process.args.push_back(process.name);
        break;
      case '3':
      	try
	{
	  maxRespawn = std::stoi(optarg);
	}
	catch(std::logic_error&)
	{
	  std::clog << LogPriority::ERR << "Invalid max respawn value!" << std::endl;
	}
        break;
      case '4':
      	try
	{
	  minRespawnDelay = std::stof(optarg);
	}
	catch(std::logic_error&)
	{
	  std::clog << LogPriority::ERR << "Invalid min respawn delay value!" << std::endl;
	}
        break;
      case 'a': /* arg */
        process.args.push_back(optarg);
        break;
      case 'e': /* env */
        process.envs.push_back(optarg);
        break;
      default: /* '?' */
        std::clog << LogPriority::EMERG << "Unknown option '" << static_cast<char>(opt) << "'" << std::endl;
        usage(bname, EXIT_FAILURE);
        break;
    }
  }
  /* sanity check */
  if(minRespawnDelay < 0)
  {
    std::clog << LogPriority::EMERG << "Invalid min respawn delay value." << std::endl;
    usage(bname, EXIT_FAILURE);
  }
  
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
  if(fileType.empty() || (fileType != XML_TYPE && fileType != JSON_TYPE))
  {
    std::clog << LogPriority::EMERG << "Invalid file type value." << std::endl;
    usage(bname, EXIT_FAILURE);
  }
  
#ifndef HAVE_XML_H
  if(fileType == XML_TYPE)
  {
    std::clog << LogPriority::EMERG << "XML not supported." << std::endl;
    usage(bname, EXIT_FAILURE);
  }
#endif /* HAVE_XML_H */
#ifndef HAVE_JSON_H
  if(fileType == JSON_TYPE)
  {
    std::clog << LogPriority::EMERG << "JSON not supported." << std::endl;
    usage(bname, EXIT_FAILURE);
  }
#endif /* HAVE_JSON_H */

#ifdef HAVE_JSON_H
  if(fileType == JSON_TYPE)
  {
    wloader = new WJSON(&process);
  }
#ifdef HAVE_XML_H
  else
#endif /* HAVE_XML_H */
#endif /* HAVE_JSON_H */
#ifdef HAVE_XML_H
  if(fileType == XML_TYPE)
  {
    wloader = new WXML(&process);
  }
#endif /* HAVE_XML_H */
  
  if(process.name.empty())
  {
    /* init the default filename */
    if(path.empty())
    {
      path = CONFIG_FILE_FOLDER;
    }
    if(path[path.length() - 1] != '/')
    {
      path.append("/");
    }

    filename.append(path);
    filename.append(bname);
    filename.append(".");
    filename.append(fileType);
    
    if(createNew) 
    {
      return create(filename);
    }
  
    /* load the config file */
    if(!wloader->load(filename))
    {
      return EXIT_FAILURE;
    }
    loadFromFile = true;
  }
#endif /* HAVE_XML_H HAVE_JSON_H */
  
  writePID(bname, pidfile, pid);
  
  /* prepare the path and find the binary */
  if(process.path.empty()) 
  {
    if(!WUtils::findFile(process.name, process.path)) 
    {
      std::clog << LogPriority::EMERG << "The BIN file '" << process.name << "' was not found." << std::endl;
      return EXIT_FAILURE;
    }
  }
  
  /* get the owner limits */
  rlimit_t ownerLimits;
  auto i = 0U;
  for(rlimit_t::iterator it = ownerLimits.begin(); it != ownerLimits.end(); ++it, i++)
    getrlimit(i, it);

  /* add the parent env variables */
  WUtils::completeEnv(&process);
  
  cmdPath = process.path;
  if(!loadFromFile)
  {    
    if(cmdPath[cmdPath.length() - 1] != '/')
      cmdPath.append("/");
    if(process.args.size() == 0)
    {
      std::clog << LogPriority::EMERG << "No given process" << std::endl;
      usage(bname, EXIT_FAILURE);
    }
    cmdPath.append(process.args[0]);
  }

  WRun run(ownerLimits, process.working);
  auto r = run.respawn(cmdPath, process.args, process.envs, disableSpamDetect, maxRespawn, minRespawnDelay);
  /* Prevents the SIGTERM */
  exit(r ? EXIT_SUCCESS : EXIT_FAILURE);
  return EXIT_SUCCESS;
}


/* Static functions ---------------------------------------------------------*/
static auto writePID(const string &bname, string &pidfile, pid_t pid) -> void
{
  if(pidfile.empty())
  {
    pidfile = PID_FOLDER + string("/") + bname + string(".pid");
  }
    
  if(pid > 0) 
  {
    fstream fs;
    fs.open(pidfile, fstream::in);
    if(fs.is_open())
    {
      std::clog << LogPriority::EMERG << "The pid file '" << pidfile << "' already exists!" << std::endl;
      fs.close();
      usage(bname, EXIT_FAILURE);
    }
    fs.open(pidfile, fstream::out);
    if(fs.is_open())
    {
      fs << pid << std::endl;
      fs.close();
    }
    else
    {
      std::clog << LogPriority::EMERG << "Unable to create the pid file '" << pidfile << "': (" << errno << ") " << strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
static auto create(const string &filename) -> int
{
  FILE* f = nullptr;
  auto sdir = filename;
  auto dir = string(dirname((char*)sdir.c_str()));
      
  if(!WUtils::mkdirs(dir))
  {
    std::clog << LogPriority::EMERG << "Unable to create directory '" << dir << "': (" << errno << ") " << strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }
      
  do 
  {
    /* Specific case in C */
    f = fopen(filename.c_str(), "wx");
    if (!f && errno == EEXIST) 
    {
      std::cout << LogPriority::EMERG << "Are you sure you want to overwrite the existing file '" << filename << "'? (y/N):" << std::endl;
      char c;
      auto r = fscanf(stdin, "%c", &c);
      (void)r; /* warning in release */
      if(c == 'y' || c == 'Y') 
      {
	unlink(filename.c_str());
      } 
      else 
	return EXIT_FAILURE;
    }
    else if(!f) 
    {
      std::clog << LogPriority::EMERG << "Unable to create the file '" << filename << "': (" << errno << ") " << strerror(errno) << std::endl;
      return EXIT_FAILURE;
    } 
    else 
      break;
  }
  while(1);
      
  if(f != nullptr)
  {
    auto s = wloader == nullptr ? nullptr : wloader->getTemplate();
    if(s == nullptr)
    {
      return EXIT_FAILURE;
    }
    fwrite(s, 1, strlen(s), f);
    fclose(f);
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}
#endif /* HAVE_XML_H HAVE_JSON_H */


static auto usage(const string &name, int err) -> void
{
  std::cout << "Usage: watchdog options" << std::endl;
  std::cout << "\t--help, -h: Print this help." << std::endl;
  std::cout << "\t--disable-spam-detect: Disable spam detection." << std::endl;
  std::cout << "\t--pid: Write the pid in the file pointed by pidfile." << std::endl;
  std::cout << "\t--pidfile: The file containing the pid (default: " << PID_FOLDER << "/" << name << ".pid) (option enables pid support)" << std::endl;
  std::cout << "\t--max-respawn: The maximum number of respawn allowed (default: " << MAX_RESPAWN << "; 0 for a start without respawn)" << std::endl;
  std::cout << "\t--min-respawn-delay: The minimum respawn delay before starting spam detection (default: " << MINIMUM_RESPAWN_DELAY << " nanosec)" << std::endl;
  std::cout << "Stand-alone mode:" << std::endl;
  std::cout << "\t--path, -p: The process path (optional if the binary is in the PATH)." << std::endl;
  std::cout << "\t--working, -w: The working directory (optional)." << std::endl;
  std::cout << "\t--name, -n: The name of the process." << std::endl;
  std::cout << "\t--arg, -a: The process argument (repeat if there're several)." << std::endl;
  std::cout << "\t--env, -e: The process environment variable (repeat if there're several)." << std::endl;
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
#if defined(HAVE_XML_H) && !defined(HAVE_JSON_H)
#define EXT "xml"
#define FT "possible value: xml"
#elif !defined(HAVE_XML_H) && defined(HAVE_JSON_H)
#define EXT "json"
#define FT "possible value: json"
#elif defined(HAVE_XML_H) || defined(HAVE_JSON_H)
#define EXT "[xml/json]"
#define FT "possible values: xml or json"
#endif /* HAVE_XML_H HAVE_JSON_H */
  std::cout << "File mode:" << std::endl;
  std::cout << "\t--directory, -d: The directory to find the configuration file (default: " << CONFIG_FILE_FOLDER << ")." << std::endl;
  std::cout << "\t--file-type, -t: The file type (" << FT << ")." << std::endl;
  std::cout << "\t--config, -c: Loads a configuration file." << std::endl;
  std::cout << "\t--new, -z: Creates a new configuration file." << std::endl;
  std::cout << "\tIf no configuration file is passed as parameter." << std::endl;
  std::cout << "\tThe application looks for a configuration file located in the " << CONFIG_FILE_FOLDER << " directory and named: <the_application_name>." << EXT << std::endl;
  std::cout << "\tIt's possible to create symbolic links with several configuration files:" << std::endl;
  std::cout << "\tln -s watchdog watchdog-foo" << std::endl;
  std::cout << "\twatchdog-foo --new" << std::endl;
  std::cout << "\tyou now can edit the configuration file " << CONFIG_FILE_FOLDER << "/watchdog-foo." << EXT << std::endl;
#endif /* HAVE_XML_H HAVE_JSON_H */
  exit(err);
}

static auto m_exit() -> void
{
#if defined(HAVE_XML_H) || defined(HAVE_JSON_H)
  if(wloader != nullptr)
  {
    delete wloader;
    wloader = nullptr;
  }
#endif /* HAVE_XML_H HAVE_JSON_H */
  
  if(pid > 0) 
  {
    std::ifstream infile;
    infile.open(pidfile);

    if (infile.is_open())
    {
      pid_t _pid; 
      infile >> _pid;
      infile.close();
      if(_pid == pid)
      {
	unlink(pidfile.c_str());
      }
      pid = 0;
    }
  }
}

static auto sig_int(int s) -> void
{ 
  exit(s);
}
