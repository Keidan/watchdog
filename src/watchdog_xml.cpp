/**
 * @file watchdog_xml.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#include "watchdog.hpp"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

constexpr const xmlChar* __XMLCAST(const char* str) 
{ 
  return reinterpret_cast<const xmlChar*>(str); 
}

#define __cleanup(doc, err) do {  \
  if(doc)                         \
    xmlFreeDoc(doc);              \
  xmlCleanupParser();             \
  if(err)                         \
    unload();                     \
}while(0)

WXML::WXML(struct watchdog_xml_s* process) : m_process(process)
{
}

WXML::~WXML()
{
  unload();
}

/**
 * @brief Loads the XML file passed in parameter.
 * @param filename The path of the xml file.
 * @return true on success false.
 */
auto WXML::load(std::string &filename) -> bool
{
  if(m_process == nullptr)
  {
    std::cerr << "Unloaded object" << std::endl;
    return false;
  }

  if(access(filename.c_str(), F_OK) != 0) 
  {
    std::cerr << "The XML file " << filename << " was not found" << std::endl;
    return false;
  } 
    
  auto doc = xmlReadFile(filename.c_str(), nullptr, 0);
  if (doc == nullptr) 
  {
    std::cerr << "Failed to parse " << filename << std::endl;
    return false;
  }
  
  /* Place the current node to the root node */
  auto curL0 = xmlDocGetRootElement(doc);
  if (curL0 == nullptr) 
  {
    std::cerr << "Empty document!" << std::endl;
    __cleanup(doc, true);
    return false;
  }

  /* Validation step */
  if (xmlStrcmp(curL0->name, __XMLCAST("process"))) 
  {
    std::cerr << "Document of the wrong type, root node != process" << std::endl;
    __cleanup(doc, true);
    return false;
  }
  
  auto attribute = curL0->properties;
  while(attribute && attribute->name && attribute->children) 
  {
    auto value = xmlNodeListGetString(curL0->doc, attribute->children, 1);
    auto v = std::string((char*)value);
    if(!xmlStrcmp(attribute->name, __XMLCAST("name"))) 
    {
      m_process->name = v;
      m_process->args.push_back(v);
    }
    xmlFree(value);
    attribute = attribute->next;
  }
  if(m_process->name.empty()) 
  {
    std::cerr << "Invalid process name" << std::endl;
    __cleanup(doc, true);
    return false;
  }

  auto curL1 = curL0->xmlChildrenNode;
  while (curL1 != nullptr) 
  {
    /* Traverse to obtain the desired node */
    if (!xmlStrcmp(curL1->name, __XMLCAST("path")))
    {
      auto value = xmlNodeGetContent(curL1);
      auto v = std::string((char*)value);
      if(v.length()) 
      {
        m_process->path = v;
        if(m_process->path[m_process->path.length() - 1] != '/')
          m_process->path.append("/");
        m_process->path.append(m_process->name);
      }
      xmlFree(value);
    } 
    else if (!xmlStrcmp(curL1->name, __XMLCAST("args")))
    {
      auto curL2 = curL1->xmlChildrenNode;
      while (curL2 != nullptr) 
      {
        if (!xmlStrcmp(curL2->name, __XMLCAST("arg")))
        {
          auto value = xmlNodeGetContent(curL2);
          auto v = std::string((char*)value);
          if(v.length()) 
          {
            m_process->args.push_back(v);
          }
          xmlFree(value);
        }
        curL2 = curL2->next;
      }
    } 
    else if (!xmlStrcmp(curL1->name, __XMLCAST("envs")))
    {
      auto curL2 = curL1->xmlChildrenNode;
      while (curL2 != nullptr) 
      {
        if (!xmlStrcmp(curL2->name, __XMLCAST("env")))
        {
          xmlChar *value = nullptr, *value2 = nullptr;
          attribute = curL2->properties;
          while(attribute && attribute->name && attribute->children) 
          {
            if(!xmlStrcmp(attribute->name, __XMLCAST("name"))) 
            {
              value = xmlNodeListGetString(curL2->doc, attribute->children, 1);
            } 
            else if(!xmlStrcmp(attribute->name, __XMLCAST("value"))) 
            {
              value2 = xmlNodeListGetString(curL2->doc, attribute->children, 1);
            }
            attribute = attribute->next;
          }
          if(value && value2) 
          {
            auto v1 = std::string((char*)value);
            auto v2 = std::string((char*)value2);
            if(v1.length() && v2.length()) 
            {
              m_process->envs.push_back(v1 + std::string("=") + v2);
            }
            if(value) xmlFree(value);
            if(value2) xmlFree(value2);
          } 
          else 
          {
            if(value) xmlFree(value);
            if(value2) xmlFree(value2);
            std::cerr << "Unable to decode the environment variable" << std::endl;
            __cleanup(doc, true);
            return false;
          }
        }
        curL2 = curL2->next;
      }
    }
    curL1 = curL1->next;
  }
  __cleanup(doc, false);
  return true;
}

/**
 * @brief Release the resources allocated by the load function.
 */
auto WXML::unload() -> void {
  if(m_process != nullptr)
  {
    m_process->args.clear();
    m_process->envs.clear();
    m_process = nullptr;
  }
}
