//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidKernel/CatalogInfo.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include <Poco/AutoPtr.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/NodeList.h>

namespace Mantid {
namespace Kernel {
/**
 * Constructor
 * @param element :: "Catalog" element from Facilities.xml
 */
CatalogInfo::CatalogInfo(const Poco::XML::Element *element) {
  m_catalogName = getAttribute(element, "catalog", "name");
  m_soapEndPoint = getAttribute(element, "soapendpoint", "url");
  m_externalDownloadURL = getAttribute(element, "externaldownload", "url");
  m_catalogPrefix = getAttribute(element, "prefix", "regex");
  m_windowsPrefix = getAttribute(element, "windows", "replacement");
  m_macPrefix = getAttribute(element, "mac", "replacement");
  m_linuxPrefix = getAttribute(element, "linux", "replacement");
}

CatalogInfo::CatalogInfo(const CatalogInfo &other)
    : m_catalogName(other.m_catalogName), m_soapEndPoint(other.m_soapEndPoint),
      m_externalDownloadURL(other.m_externalDownloadURL),
      m_catalogPrefix(other.m_catalogPrefix),
      m_windowsPrefix(other.m_windowsPrefix), m_macPrefix(other.m_macPrefix),
      m_linuxPrefix(other.m_linuxPrefix) {}

/**
 * Obtain catalog name from the facility file.
 */
const std::string CatalogInfo::catalogName() const { return (m_catalogName); }

/**
 * Obtain soap end point from the facility file.
 */
const std::string CatalogInfo::soapEndPoint() const { return (m_soapEndPoint); }

/**
 * Obtain catalog name from the facility file.
 */
const std::string CatalogInfo::externalDownloadURL() const {
  return (m_externalDownloadURL);
}

/**
 * Obtain the regex prefix for default archive path.
 */
const std::string CatalogInfo::catalogPrefix() const {
  return (m_catalogPrefix);
}

/**
 * Obtain Windows prefix from the facility file.
 */
const std::string CatalogInfo::windowsPrefix() const {
  return (m_windowsPrefix);
}

/**
 * Obtain Macintosh prefix from facility file.
 */
const std::string CatalogInfo::macPrefix() const { return (m_macPrefix); }

/**
 * Obtain Linux prefix from facility file.
 */
const std::string CatalogInfo::linuxPrefix() const { return (m_linuxPrefix); }

/**
 * Transform's the archive path based on operating system (OS) used.
 * @param path :: The archive path from ICAT to perform the transform on.
 * @return The path to the archive for the user's OS.
 */
std::string CatalogInfo::transformArchivePath(const std::string &path) const {
  std::string ret;
#ifdef __linux__
  ret = replacePrefix(path, catalogPrefix(), linuxPrefix());
  ret = replaceAllOccurences(ret, "\\", "/");
#elif __APPLE__
  ret = replacePrefix(path, catalogPrefix(), macPrefix());
  ret = replaceAllOccurences(ret, "\\", "/");
#elif _WIN32
  // Check to see if path is a windows path.
  if (path.find("\\") == std::string::npos) {
    ret = replacePrefix(path, linuxPrefix(), windowsPrefix());
    ret = replaceAllOccurences(ret, "/", "\\");
  }
#endif
  return ret;
}

/**
 * Replace the content of a string using regex.
 * @param path   :: An string to search and replace on.
 * @param regex  :: The regex to search for.
 * @param prefix :: Replace result of regex with this prefix.
 * @return A string containing the replacement.
 */
std::string CatalogInfo::replacePrefix(const std::string &path,
                                       const std::string &regex,
                                       const std::string &prefix) const {
  boost::regex re(regex);
  // Assign the result of the replacement back to path and return it.
  return boost::regex_replace(path, re, prefix);
}

/**
 * Replace all occurrences of the search string in the input with the format
 * string.
 * @param path    :: An string to search and replace on.
 * @param search  :: A substring to be searched for.
 * @param format  :: A substitute string.
 * @return A string containing the replacement.
 */
std::string CatalogInfo::replaceAllOccurences(const std::string &path,
                                              const std::string &search,
                                              const std::string &format) const {

  return boost::replace_all_copy(path, search, format);
}

/**
 * Obtain the attribute from a given element tag and attribute name.
 * @param element       :: The name of the element in the XML file.
 * @param tagName       :: The name of the tag to search for.
 * @param attributeName :: The name of the attribute for the given tag.
 * @return The contents of the attribute from an XML element.
 */
std::string CatalogInfo::getAttribute(const Poco::XML::Element *element,
                                      const std::string &tagName,
                                      const std::string &attributeName) {
  Poco::AutoPtr<Poco::XML::NodeList> elementTag =
      element->getElementsByTagName(tagName);

  // If the tag exists in the XML file.
  if (elementTag->length() == 1) {
    Poco::XML::Element *item =
        dynamic_cast<Poco::XML::Element *>(elementTag->item(0));

    // If the item does exist, then we want to return it.
    if (!item->getAttribute(attributeName).empty()) {
      return (item->getAttribute(attributeName));
    }
  }
  return ("");
}

} // namespace Kernel
} // namespace Mantid
