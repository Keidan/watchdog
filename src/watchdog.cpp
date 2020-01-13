/**
 * @file watchdog.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#include "watchdog.hpp"
#include <signal.h>
#include <getopt.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

static std::string pidfile = "";
static pid_t pid = -1;
static std::string filename = "";
static struct watchdog_xml_s xml;
static WXML wxml(&xml);


static const struct option long_options[] = { 
  { "help"                   , 0, NULL, 'h' },
  { "config"                 , 1, NULL, 'c' },
  { "directory"              , 1, NULL, 'd' },
  { "new"                    , 0, NULL, 'z' },
  { "path"                   , 1, NULL, 'p' },
  { "name"                   , 1, NULL, 'n' },
  { "arg"                    , 1, NULL, 'a' },
  { "env"                    , 1, NULL, 'e' },
  { "pid"                    , 0, NULL, '0' },
  { "pidfile"                , 1, NULL, '1' },
  { "disable-spam-detect"    , 0, NULL, '2' },
  { NULL                     , 0, NULL, 0   } 
};

static auto m_exit(void) -> void;
static auto sig_int(int s) -> void;
static auto usage(std::string &name, int err) -> void;


auto main(int argc, char** argv) -> int
{
  struct sigaction sa;
  bool z = false;
  std::string path = "";
  std::string cmd_path = "";
  bool disableSpamDetect = false;
  bool loadFromXML = false;

  xml.name = "";
  xml.path = "";
  
  atexit(m_exit);
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sig_int;
  (void)sigaction(SIGINT, &sa, NULL);
  (void)sigaction(SIGTERM, &sa, NULL);


  auto bname = std::string(basename(argv[0]));
  
  /* parse the arguments */
  int opt;
  while ((opt = getopt_long(argc, argv, "hc:zd:n:a:e:p:01:2", long_options, NULL)) != -1) 
  {
    switch (opt) 
    {
      case 'h': 
        usage(bname, 0); 
        break;
      case 'c': /* configuration file */
        filename = std::string(optarg);
        break;
      case 'd': /* directory */
        path = std::string(optarg);
        break;
      case '2': /* disable-spam-detect */
        disableSpamDetect = true;
        break;
      case 'z': /* new */
        z = true;
        break;
      case '0': /* pid */
        pid = getpid();
        break;
      case '1': /* pidfile */
        pidfile = std::string(optarg);
        pid = getpid();
        break;
      case 'p': /* path */
        xml.path = std::string(optarg);
        break;
      case 'n': /* name */
        xml.name = std::string(optarg);
        xml.args.push_back(xml.name);
        break;
      case 'a': /* arg */
        xml.args.push_back(optarg);
        break;
      case 'e': /* env */
        xml.envs.push_back(optarg);
        break;
      default: /* '?' */
        std::cerr << "Unknown option '" << opt << "'" << std::endl;
        usage(bname, EXIT_FAILURE);
        break;
    }
  }

  if(pidfile.empty())  
    pidfile = PID_FOLDER + std::string("/") + bname + std::string(".pid");
    
  if(pid > 0) 
  {
    std::fstream fs;
    fs.open(pidfile, std::fstream::in);
    if(fs.is_open())
    {
      std::cerr << "The pid file '" << pidfile << "' already exists!" << std::endl;
      fs.close();
      usage(bname, EXIT_FAILURE);
    }
    fs.open(pidfile, std::fstream::out);
    if(fs.is_open())
    {
      fs << pid << std::endl;
      fs.close();
    }
    else
    {
      std::cerr << "Unable to create the pid file '" << pidfile << "': (" << errno << ") " << strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  
  if(xml.name.empty())
  {
    /* init the default filename */
    if(path.empty())
      path = CONFIG_FILE_FOLDER;
    if(path[path.length() - 1] != '/')
      path.append("/");

    filename.append(path);
    filename.append(bname);
    filename.append(".xml");
    
    if(z) 
    {
      FILE* f = nullptr;
      struct stat info;
      auto sdir = filename;
      auto dir = dirname((char*)sdir.c_str());
      if(stat(dir, &info) != 0)
      {
	std::cerr << "Cannot acces to " << dir << std::endl;
	return EXIT_FAILURE;
      }
      else if(!(info.st_mode & S_IFDIR))
      {
	if(mkdir(dir, 0777) == -1)
	{
	  std::cerr << "Unable to create directory '" << dir << "': (" << errno << ") " << strerror(errno) << std::endl;
	  return EXIT_FAILURE;
	}
      }
      do 
      {
	/* Specific case in C */
	f = fopen(filename.c_str(), "wx");
	if (!f && errno == EEXIST) 
	{
	  std::cerr << "Are you sure you want to overwrite the existing file '" << filename << "'? (y/N):" << std::endl;
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
	  std::cerr << "Unable to create the file '" << filename << "': (" << errno << ") " << strerror(errno) << std::endl;
	  return EXIT_FAILURE;
	} 
	else 
	  break;
      }
      while(1);
      
      if(f != nullptr)
      {
        std::string s = EMPTY_CONFIG_FILE;
        fwrite(s.c_str(), 1, s.length(), f);
        fclose(f);
        return EXIT_SUCCESS;
      }
      return EXIT_FAILURE;
    }
  
    /* load the config file */
    if(!wxml.load(filename))
      return EXIT_FAILURE;
    loadFromXML = true;
  }
  
  /* prepare the path and find the binary */
  if(xml.path.empty()) 
  {
    if(!WUtils::findFile(xml.name, xml.path)) 
    {
      std::cerr << "The BIN file '" << xml.path << "' was not found." << std::endl;
      return EXIT_FAILURE;
    }
  }
  
  /* get the owner limits */
  rlimit_t ownerLimits;
  auto i = 0U;
  for(rlimit_t::iterator it = ownerLimits.begin(); it != ownerLimits.end(); ++it, i++)
    getrlimit(i, it);

  /* add the parent env variables */
  WUtils::completeEnv(&xml);
  
  cmd_path = xml.path;
  if(!loadFromXML)
  {    
    if(cmd_path[cmd_path.length() - 1] != '/')
      cmd_path.append("/");
    cmd_path.append(xml.args[0]);
  }

  WRun run(ownerLimits);
  auto r = run.respawn(cmd_path, xml.args, xml.envs, disableSpamDetect);
  
  return r ? EXIT_SUCCESS : EXIT_FAILURE;
}

static auto usage(std::string &name, int err) -> void
{
  std::cout << "usage: watchdog options" << std::endl;
  std::cout << "\t--help, -h: Print this help." << std::endl;
  std::cout << "\t--disable-spam-detect: Disable the spamming detection." << std::endl;
  std::cout << "\t--pid: Write the pid into the file pointed by pidfile." << std::endl;
  std::cout << "\t--pidfile: The file containing the pid (default: " << PID_FOLDER << "/" << name << ".pid) (this option enable the support of the pid)" << std::endl;
  std::cout << "Mode standalone:" << std::endl;
  std::cout << "\t--path, -p: The process path (optional if the binary is in the PATH)." << std::endl;
  std::cout << "\t--name, -n: The process name." << std::endl;
  std::cout << "\t--arg, -a: The process argument (repeat for more)." << std::endl;
  std::cout << "\t--env, -e: The process environment variable (repeat for more)." << std::endl;
  std::cout << "Mode file:" << std::endl;
  std::cout << "\t--directory, -d: The directory to search the configuration file (default:" << CONFIG_FILE_FOLDER << ")." << std::endl;
  std::cout << "\t--config, -c: Load a config file." << std::endl;
  std::cout << "\t--new, -z: Create a new config file." << std::endl;
  std::cout << "\tIf no configuration file is passed as parameter." << std::endl;
  std::cout << "\tThe application search a configuration file localized into the folder " << CONFIG_FILE_FOLDER << " and named: <the_application_name>.xml" << std::endl;
  std::cout << "\tIt's possible to create symbolic links with several configuration files:" << std::endl;
  std::cout << "\tln -s watchdog watchdog-foo" << std::endl;
  std::cout << "\twatchdog-foo --new" << std::endl;
  std::cout << "\tnow you can edit the config file " << CONFIG_FILE_FOLDER << "/watchdog-foo.xml" << std::endl;
  exit(err);
}

static auto m_exit() -> void
{
  wxml.unload();
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
          unlink(pidfile.c_str());
        pid = 0;
    }
  }
}

static auto sig_int(int s) -> void
{ 
  exit(s);
}
