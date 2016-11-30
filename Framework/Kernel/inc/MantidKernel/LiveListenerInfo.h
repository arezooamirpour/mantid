#ifndef MANTID_KERNEL_LIVELISTENERINFO_H_
#define MANTID_KERNEL_LIVELISTENERINFO_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidKernel/DllConfig.h"
#include <string>

//----------------------------------------------------------------------
// Forward declarations
//----------------------------------------------------------------------
namespace Poco {
namespace XML {
class Element;
}
}

namespace Mantid {
namespace Kernel {

//----------------------------------------------------------------------
// Forward declarations
//----------------------------------------------------------------------
class InstrumentInfo;

/**
 * A class that holds information about a LiveListener connection.
 *
 * Copyright &copy; 2016 STFC Rutherford Appleton Laboratory
 *
 * This file is part of Mantid.
 *
 * Mantid is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mantid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * File change history is stored at: <https://github.com/mantidproject/mantid>.
 * Code Documentation is available at: <http://doxygen.mantidproject.org>
 */
class MANTID_KERNEL_DLL LiveListenerInfo {
public:
  LiveListenerInfo(InstrumentInfo* inst, const Poco::XML::Element *elem);
  LiveListenerInfo(const std::string &listener = "",
                   const std::string &address = "",
                   const std::string &name = "");

  /// Required for Python bindings
  bool operator==(const LiveListenerInfo &rhs) const;

  /// Return the name of this LiveListener connection
  const std::string &name() const;

  /// Returns the address string of this LiveListener connection
  const std::string &address() const;

  /// Returns the classname of the specific LiveListener to use
  const std::string &listener() const;

private:
  std::string m_name;     ///< Listener name
  std::string m_address;  ///< Listener address
  std::string m_listener; ///< Listener classname
};

/// Allow this object to be printed to a stream
MANTID_KERNEL_DLL std::ostream &operator<<(std::ostream &buffer,
                                           const LiveListenerInfo &listener);

} // namespace Kernel
} // namespace Mantid

#endif /* MANTID_KERNEL_LIVELISTENERINFO_H_ */
