/**
 * @file ConfigJSON.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
/* Includes -----------------------------------------------------------------*/
#include <wd/config/ConfigJSON.hpp>
#ifndef HAVE_JSON_H
#  error "nlohmann-json not found!"
#endif /* HAVE_JSON_H */
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <wd/utils/Helper.hpp>
#include <wd/utils/Log.hpp>

/* Usings -------------------------------------------------------------------*/
using namespace wd::config;

using wd::utils::Helper;
using wd::utils::LogPriority;
using json = nlohmann::json;

/* Private functions --------------------------------------------------------*/

/* Private defines ----------------------------------------------------------*/
static constexpr auto* PROCESS = "process";

/* Public functions ---------------------------------------------------------*/
ConfigJSON::ConfigJSON(std::shared_ptr<run::Process> process) : m_process(std::move(process)) {}

/**
 * @brief Returns the file template.
 *
 * @retval std::string
 */
auto ConfigJSON::getTemplate() -> std::string
{
  std::string ret = "{\n";
  ret.append("  \"process\": { \n");
  ret.append("    // The process name eg with /bin/ls name=\"ls\"\n");
  ret.append("    \"name\": \"foo\",\n");
  ret.append("    // Optional, if the file is not in the PATH\n");
  ret.append("    \"path\": \"\",\n");
  ret.append("    // Optional, application working directory\n");
  ret.append("    \"working\": \"\",\n");
  ret.append("    // Optional, arguments list eg: \"-a\", \"-l\", etc\n");
  ret.append("    \"args\": [\n");
  ret.append("    ],\n");
  ret.append("    // Optional, environment variables list\n");
  ret.append("    \"envs\": [\n");
  ret.append("      {\"name\": \"\", \"value\": \"\"}\n");
  ret.append("    ]\n");
  ret.append("  }\n");
  ret.append("}\n");
  return ret;
}

/**
 * @brief Loads the XML file passed in parameter.
 *
 * @param[in] filename The path of the xml file.
 * @retval false on error, otherwise true.
 */
auto ConfigJSON::load(const std::string& filename) -> bool
{
  std::ifstream ifs;

  if (nullptr == m_process)
  {
    std::clog << LogPriority::ERR << "Unloaded object" << std::endl;
    return false;
  }

  ifs.open(filename);
  if (!ifs.is_open() || ifs.fail())
  {
    std::clog << LogPriority::ERR << "The JSON file " << filename << " was not found" << std::endl;
    return false;
  }
  try
  {
    json data = json::parse(ifs, nullptr, true, true);
    /* Validation step */
    if (!data.contains(PROCESS))
    {
      std::clog << LogPriority::ERR << "Wrong type, root node != " << PROCESS << std::endl;
      return false;
    }

    if (data[PROCESS].contains("name"))
    {
      m_process->setName(data[PROCESS]["name"]);
      m_process->getArgs().emplace_back(m_process->getName());
    }
    if (m_process->getName().empty())
    {
      std::clog << LogPriority::ERR << "Invalid process name" << std::endl;
      return false;
    }

    if (data[PROCESS].contains("path"))
    {
      m_process->setPath(data[PROCESS]["path"]);
    }
    if (data[PROCESS].contains("working"))
    {
      m_process->setWorking(data[PROCESS]["working"]);
    }
    if (data[PROCESS].contains("args"))
    {
      auto args = data[PROCESS]["args"];
      for (auto& element : args)
      {
        m_process->getArgs().emplace_back(element);
      }
    }
    if (data[PROCESS].contains("envs"))
    {
      auto envs = data[PROCESS]["envs"];
      for (auto& element : envs)
      {
        if (element.contains("name") && element.contains("value"))
        {
          addNameValue(element["name"], element["value"]);
        }
      }
    }
  }
  catch (json::exception& e)
  {
    std::clog << LogPriority::ERR << e.what() << std::endl;
    return false;
  }

  return true;
}

auto ConfigJSON::addNameValue(const std::string& name, const std::string& value) const -> void
{
  if (!name.empty())
  {
    m_process->getEnvs().emplace_back(name + std::string("=") + value);
  }
}