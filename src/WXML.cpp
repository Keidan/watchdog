/**
 * @file watchdog_xml.cpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
/* Includes -----------------------------------------------------------------*/
#include "watchdog.hpp"
#ifndef HAVE_XML_H
#error "libxml not found!"
#endif /* HAVE_XML_H */
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

/* Usings -------------------------------------------------------------------*/
using std::string;
using std::clog;
using std::endl;


/* Private functions --------------------------------------------------------*/
#define __XMLCAST(str) reinterpret_cast<const xmlChar*>(str)
#define __CXXCAST(xml) string(reinterpret_cast<char*>(xml))

/* Private macros -----------------------------------------------------------*/
#define __cleanup(doc) do {			\
    if(doc)					\
      xmlFreeDoc(doc);				\
    xmlCleanupParser();				\
  }while(0)


/* Public functions ---------------------------------------------------------*/
WXML::WXML(struct WProcess* process) : WLoader(process)
{
}


/**
 * @brief Returns the file template.
 * @retval const char*
 */
auto WXML::getTemplate() -> const char*
{
  return "\
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
</process>\n";
}

/**
 * @brief Loads the XML file passed in parameter.
 * @param[in] filename The path of the xml file.
 * @retval true on error, otherwise false.
 */
auto WXML::load(const string &filename) -> bool
{
  if(m_process == nullptr)
  {
    clog << LogPriority::ERR << "Unloaded object" << endl;
    return false;
  }

  if(access(filename.c_str(), F_OK) != 0) 
  {
    clog << LogPriority::ERR << "The XML file " << filename << " was not found" << endl;
    return false;
  } 
    
  auto doc = xmlReadFile(filename.c_str(), nullptr, 0);
  if (doc == nullptr) 
  {
    clog << LogPriority::ERR << "Failed to parse " << filename << endl;
    return false;
  }
  
  /* Place the current node to the root node */
  auto curL0 = xmlDocGetRootElement(doc);
  if (curL0 == nullptr) 
  {
    clog << LogPriority::ERR << "Empty document!" << endl;
    __cleanup(doc);
    return false;
  }

  /* Validation step */
  if (xmlStrcmp(curL0->name, __XMLCAST("process"))) 
  {
    clog << LogPriority::ERR << "Wrong type, root node != process" << endl;
    __cleanup(doc);
    return false;
  }
  
  auto attribute = curL0->properties;
  while(attribute && attribute->name && attribute->children) 
  {
    auto value = xmlNodeListGetString(curL0->doc, attribute->children, 1);
    auto v = string((char*)value);
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
    clog << LogPriority::ERR << "Invalid process name" << endl;
    __cleanup(doc);
    return false;
  }

  auto curL1 = curL0->xmlChildrenNode;
  while (curL1 != nullptr) 
  {
    /* Traverse to obtain the desired node */
    if (!xmlStrcmp(curL1->name, __XMLCAST("path")))
    {
      auto value = xmlNodeGetContent(curL1);
      auto v = __CXXCAST(value);
      if(v.length()) 
      {
	m_process->path = v;
	if(m_process->path[m_process->path.length() - 1] != '/')
	{
	  m_process->path.append("/");
	}
	m_process->path.append(m_process->name);
      }
      xmlFree(value);
    } 
    else if (!xmlStrcmp(curL1->name, __XMLCAST("working")))
    {
      auto value = xmlNodeGetContent(curL1);
      auto v = __CXXCAST(value);
      if(v.length()) 
      {
	m_process->working = v;
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
	  auto v = __CXXCAST(value);
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
	    auto v1 = __CXXCAST(value);
	    auto v2 = __CXXCAST(value2);
	    if(v1.length() && v2.length()) 
	    {
	      m_process->envs.push_back(v1 + string("=") + v2);
	    }
	    if(value)
	    {
	      xmlFree(value);
	    }
	    if(value2)
	    {
	      xmlFree(value2);
	    }
	  } 
	  else 
	  {
	    if(value)
	    {
	      xmlFree(value);
	    }
	    if(value2)
	    {
	      xmlFree(value2);
	    }
	    clog << LogPriority::ERR << "Unable to decode the environment variable" << endl;
	    __cleanup(doc);
	    return false;
	  }
	}
	curL2 = curL2->next;
      }
    }
    curL1 = curL1->next;
  }
  __cleanup(doc);
  return true;
}

