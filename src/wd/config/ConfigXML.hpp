/**
 * @file ConfigXML.hpp
 * @author Keidan
 * @copyright GNU GENERAL PUBLIC LICENSE Version 3
 */
#pragma once

/* Includes -----------------------------------------------------------------*/
#include <memory>
#include <wd/BuildConfig.hpp>
#include <wd/config/IConfigable.hpp>
#include <wd/run/Process.hpp>

#ifdef HAVE_XML_H
#  include <libxml/parser.h>
#  include <libxml/xmlmemory.h>

/* Class --------------------------------------------------------------------*/
namespace wd::config
{
  /****************
   * XML
   ****************/
  class ConfigXML : public IConfigable
  {
    public:
      explicit ConfigXML(std::shared_ptr<run::Process> process);
      ~ConfigXML() override = default;

      /**
       * @brief Loads the file passed in parameter.
       *
       * @param[in] filename The path of the file.
       * @retval false on error, otherwise true.
       */
      auto load(const std::string& filename) -> bool override;

      /**
       * @brief Returns the file template.
       *
       * @retval std::string
       */
      auto getTemplate() -> std::string override;

    private:
      std::shared_ptr<run::Process> m_process = nullptr;

      /**
       * @brief Cleanup the XML ctx.
       *
       * @param[in,out] doc The document.
       */
      static auto cleanup(xmlDocPtr& doc) -> void;

      /**
       * @brief Converts an XML string to C++.
       *
       * @param[in] The string to convert.
       * @retval The converted string.
       */
      static auto toCXX(const xmlChar* xml) -> std::string;

      /**
       * @brief Converts a C++ string to XML.
       *
       * @param[in] The string to convert.
       * @retval The converted string.
       */
      static auto toXML(const char* cxx) -> const xmlChar*;

      /**
       * @brief Initializes the xml context.
       *
       * @param[out] doc The document.
       * @param[in] filename The file to load.
       * @retval false on error, otherwise true.
       */
      auto initializeXML(xmlDocPtr& doc, const char* filename) const -> bool;

      /**
       * @brief Extracts the env.
       *
       * @param[in,out] attribute The current attribute.
       * @param[in,out] doc The current node document.
       * @retval false on error, otherwise true.
       */
      auto extractEnv(_xmlAttr** attribute, xmlDocPtr& doc) const -> bool;

      /**
       * @brief Extracts the envs.
       *
       * @param[in,out] node The current node.
       * @param[in,out] doc The root node document.
       * @retval false on error, otherwise true.
       */
      auto extractEnvs(_xmlNode* node, xmlDocPtr& doc) const -> bool;

      /**
       * @brief Extracts the args.
       *
       * @param[in,out] node The current node.
       */
      auto extracArgs(_xmlNode* node) const -> void;
  };
} // namespace wd::config
#endif /* HAVE_XML_H */