/**
 * @file watchdog_json.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
/* Includes -----------------------------------------------------------------*/
#include "watchdog.hpp"
#ifndef HAVE_JSON_H
#error "libjsoncpp not found!"
#endif /* HAVE_JSON_H */
#include <json/json.h>
#include <fstream>
/* Usings -------------------------------------------------------------------*/
using std::string;
using std::clog;
using std::endl;
using std::ifstream;


/* Private functions --------------------------------------------------------*/

/* Private defines ----------------------------------------------------------*/
#define PROCESS "process"
/* Public functions ---------------------------------------------------------*/
WJSON::WJSON(struct WProcess* process) : WLoader(process)
{
}


/**
 * @brief Returns the file template.
 * @retval const char*
 */
auto WJSON::getTemplate() -> const char*
{
  return "{\n\
  \"process\": { \n\
    // The process name eg with /bin/ls name=\"ls\"\n\
    \"name\": \"foo\",\n\
    // Optional, if the file is not in the PATH\n\
    \"path\": \"\",\n\
    // Optional, application working directory\n\
    \"working\": \"\",\n\
    // Optional, arguments list eg: \"-a\", \"-l\", etc\n\
    \"args\": [\n\
    ],\n\
    // Optional, environment variables list\n\
    \"envs\": [\n\
      {\"name\": \"\", \"value\": \"\"}\n\
    ]\n\
  }\n\
}\n";
}


/**
 * @brief Loads the XML file passed in parameter.
 * @param[in] filename The path of the xml file.
 * @retval true on error, otherwise false.
 */
auto WJSON::load(const string &filename) -> bool
{
  Json::Value root;
  Json::CharReaderBuilder builder;
  JSONCPP_STRING errs;
  ifstream ifs;
  
  if(m_process == nullptr)
  {
    clog << LogPriority::ERR << "Unloaded object" << endl;
    return false;
  }

  ifs.open(filename);
  if(!ifs.is_open() || ifs.fail())
  {
    clog << LogPriority::ERR << "The JSON file " << filename << " was not found" << endl;
    return false;
  }
  builder["collectComments"] = true;
  
  if (!parseFromStream(builder, ifs, &root, &errs))
  {
    clog << LogPriority::ERR << errs << std::endl;
    return false;
  }

  /* Validation step */
  if (!root.isMember(PROCESS)) 
  {
    clog << LogPriority::ERR << "Wrong type, root node != " << PROCESS << endl;
    return false;
  }
  
  
  if(root[PROCESS].isMember("name"))
  {
    m_process->name = root[PROCESS]["name"].asString();
    m_process->args.push_back(m_process->name);
  }
  if(m_process->name.empty()) 
  {
    clog << LogPriority::ERR << "Invalid process name" << endl;
    return false;
  }

  if(root[PROCESS].isMember("path"))
  {
    m_process->path = root[PROCESS]["path"].asString();
    if(!m_process->path.empty())
    {
      if(m_process->path[m_process->path.length() - 1] != '/')
      {
	m_process->path.append("/");
      }
      m_process->path.append(m_process->name);
    }
  }
  if(root[PROCESS].isMember("working"))
  {
    m_process->working = root[PROCESS]["working"].asString();
  }
  if(root[PROCESS].isMember("args"))
  {
    auto args = root[PROCESS]["args"]; 
    for(int i = 0L; i < static_cast<int>(args.size()); ++i)
    {
      auto s = args[i].asString();
      if(!s.empty())
      {
	m_process->args.push_back(s);
      }
    }
  }
  if(root[PROCESS].isMember("envs"))
  {
    auto envs = root[PROCESS]["envs"]; 
    for(int i = 0L; i < static_cast<int>(envs.size()); ++i)
    {
      auto env = envs[i];
      if(env.isMember("name") && env.isMember("value"))
      {
	auto sn = env["name"].asString();
	auto sv = env["value"].asString();
	if(!sn.empty())
	{
	  m_process->envs.push_back(sn + string("=") + sv);
	}
      }
    }
  }
  return true;
}

