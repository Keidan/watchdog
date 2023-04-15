/**
 * @file ConfigXML.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
/* Includes -----------------------------------------------------------------*/
#include <wd/config/ConfigXML.hpp>
#ifndef HAVE_XML_H
#  error "libxml not found!"
#endif /* HAVE_XML_H */
#include <filesystem>
#include <iostream>
#include <wd/utils/Helper.hpp>
#include <wd/utils/Log.hpp>

/* Usings -------------------------------------------------------------------*/
using namespace wd::config;
namespace fs = std::filesystem;

using wd::utils::Helper;
using wd::utils::LogPriority;

/* Private functions --------------------------------------------------------*/

/* Private macros -----------------------------------------------------------*/

/* Public functions ---------------------------------------------------------*/
ConfigXML::ConfigXML(std::shared_ptr<run::Process> process) : m_process(std::move(process)) {}

/**
 * @brief Converts an XML string to C++.
 *
 * @param[in] The string to convert.
 * @retval The converted string.
 */
auto ConfigXML::toCXX(const xmlChar* xml) -> std::string { return std::string((const char*)(xml)); }

/**
 * @brief Converts a C++ string to XML.
 *
 * @param[in] The string to convert.
 * @retval The converted string.
 */
auto ConfigXML::toXML(const char* cxx) -> const xmlChar* { return (const xmlChar*)(cxx); }

/**
 * @brief Cleanup the XML ctx.
 *
 * @param[in,out] doc The document.
 */
auto ConfigXML::cleanup(xmlDocPtr& doc) -> void
{
  if (doc)
    xmlFreeDoc(doc);
  xmlCleanupParser();
}

/**
 * @brief Returns the file template.
 *
 * @retval std::string
 */
auto ConfigXML::getTemplate() -> std::string
{
  std::string ret = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
  ret.append("<!-- The process name eg with /bin/ls name=\"ls\" -->\n");
  ret.append("<process name=\"foo\">\n");
  ret.append("  <!-- Optional, if the file is not in the PATH -->\n");
  ret.append("  <path></path>\n");
  ret.append("  <!-- Optional, application working directory -->\n");
  ret.append("  <working></working>\n");
  ret.append("  <args>\n");
  ret.append("    <!-- Optional, arguments list -->\n");
  ret.append("    <arg></arg>\n");
  ret.append("  </args>\n");
  ret.append("  <envs>\n");
  ret.append("    <!-- Optional, environment variables list -->\n");
  ret.append("    <env name=\"\" value=\"\"/>\n");
  ret.append("  </envs>\n");
  ret.append("</process>\n");
  return ret;
}

/**
 * @brief Initializes the xml context.
 *
 * @param[out] doc The document.
 * @param[in] filename The file to load.
 * @retval true on error, otherwise false.
 */
auto ConfigXML::initializeXML(xmlDocPtr& doc, const char* filename) const -> bool
{
  if (nullptr == m_process)
  {
    std::clog << LogPriority::ERR << "Unloaded object" << std::endl;
    return false;
  }

  if (!fs::exists(filename))
  {
    std::clog << LogPriority::ERR << "The XML file " << filename << " was not found" << std::endl;
    return false;
  }

  doc = xmlReadFile(filename, nullptr, 0);
  if (nullptr == doc)
  {
    std::clog << LogPriority::ERR << "Failed to parse " << filename << std::endl;
    return false;
  }
  return true;
}

/**
 * @brief Loads the XML file passed in parameter.
 *
 * @param[in] filename The path of the xml file.
 * @retval false on error, otherwise true.
 */
auto ConfigXML::load(const std::string& filename) -> bool
{
  xmlDocPtr doc;

  if (!initializeXML(doc, filename.c_str()))
    return false;

  /* Place the current node to the root node */
  auto curL0 = xmlDocGetRootElement(doc);
  if (nullptr == curL0)
  {
    std::clog << LogPriority::ERR << "Empty document!" << std::endl;
    cleanup(doc);
    return false;
  }

  /* Validation step */
  if (xmlStrcmp(curL0->name, toXML("process")))
  {
    std::clog << LogPriority::ERR << "Wrong type, root node != process" << std::endl;
    cleanup(doc);
    return false;
  }

  auto attribute = curL0->properties;
  while (attribute && attribute->name && attribute->children)
  {
    auto value = xmlNodeListGetString(curL0->doc, attribute->children, 1);
    auto v = std::string((char*)value);
    if (!xmlStrcmp(attribute->name, toXML("name")))
    {
      m_process->setName(v);
      m_process->getArgs().emplace_back(v);
    }
    xmlFree(value);
    attribute = attribute->next;
  }
  if (m_process->getName().empty())
  {
    std::clog << LogPriority::ERR << "Invalid process name" << std::endl;
    cleanup(doc);
    return false;
  }

  auto curL1 = curL0->xmlChildrenNode;
  while (nullptr != curL1)
  {
    /* Traverse to obtain the desired node */
    if (!xmlStrcmp(curL1->name, toXML("path")))
    {
      auto value = xmlNodeGetContent(curL1);
      if (auto v = toCXX(value); v.length())
      {
        m_process->setPath(v);
      }
      xmlFree(value);
    }
    else if (!xmlStrcmp(curL1->name, toXML("working")))
    {
      auto value = xmlNodeGetContent(curL1);
      if (auto v = toCXX(value); v.length())
      {
        m_process->setWorking(v);
      }
      xmlFree(value);
    }
    else if (!xmlStrcmp(curL1->name, toXML("args")))
    {
      extracArgs(curL1->xmlChildrenNode);
    }
    else if (!xmlStrcmp(curL1->name, toXML("envs")))
    {
      extractEnvs(curL1->xmlChildrenNode, doc);
    }
    curL1 = curL1->next;
  }
  cleanup(doc);
  return true;
}
/**
 * @brief Extracts the args.
 *
 * @param[in,out] node The current node.
 */
auto ConfigXML::extracArgs(_xmlNode* node) const -> void
{
  auto curL2 = node;
  while (nullptr != curL2)
  {
    if (!xmlStrcmp(curL2->name, toXML("arg")))
    {
      auto value = xmlNodeGetContent(curL2);
      if (auto v = toCXX(value); v.length())
      {
        m_process->getArgs().emplace_back(v);
      }
      xmlFree(value);
    }
    curL2 = curL2->next;
  }
}

/**
 * @brief Extracts the envs.
 *
 * @param[in,out] node The current node.
 * @param[in,out] doc The root node document.
 * @retval false on error, otherwise true.
 */
auto ConfigXML::extractEnvs(_xmlNode* node, xmlDocPtr& doc) const -> bool
{
  auto curL2 = node;
  while (nullptr != curL2)
  {
    if (!xmlStrcmp(curL2->name, toXML("env")))
    {
      _xmlAttr* attribute = curL2->properties;
      if (!extractEnv(&attribute, curL2->doc))
      {
        std::clog << LogPriority::ERR << "Unable to decode the environment variable" << std::endl;
        cleanup(doc);
        return false;
      }
    }
    curL2 = curL2->next;
  }
  return true;
}
/**
 * @brief Extracts the env.
 *
 * @param[in,out] attribute The current attribute.
 * @param[in,out] doc The current node document.
 * @retval false on error, otherwise true.
 */
auto ConfigXML::extractEnv(_xmlAttr** attribute, xmlDocPtr& doc) const -> bool
{
  xmlChar* value = nullptr;
  xmlChar* value2 = nullptr;
  while ((*attribute) && (*attribute)->name && (*attribute)->children)
  {
    if (!xmlStrcmp((*attribute)->name, toXML("name")))
    {
      value = xmlNodeListGetString(doc, (*attribute)->children, 1);
    }
    else if (!xmlStrcmp((*attribute)->name, toXML("value")))
    {
      value2 = xmlNodeListGetString(doc, (*attribute)->children, 1);
    }
    (*attribute) = (*attribute)->next;
  }
  if (value && value2)
  {
    auto v1 = toCXX(value);
    if (auto v2 = toCXX(value2); v1.length() && v2.length())
    {
      m_process->getEnvs().emplace_back(v1 + std::string("=") + v2);
    }
    if (value)
    {
      xmlFree(value);
    }
    if (value2)
    {
      xmlFree(value2);
    }
  }
  else
  {
    if (value)
    {
      xmlFree(value);
    }
    if (value2)
    {
      xmlFree(value2);
    }
    return false;
  }
  return true;
}