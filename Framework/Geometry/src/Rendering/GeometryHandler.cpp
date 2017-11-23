#include "MantidGeometry/Rendering/GeometryHandler.h"
#include "MantidKernel/make_unique.h"

namespace Mantid {
namespace Geometry {

/** Constructor
 *  @param[in] comp
 *  This geometry handler will be ObjComponent's geometry handler
 */
GeometryHandler::GeometryHandler(IObjComponent *comp)
    : Obj() {
  ObjComp = comp;
  boolTriangulated = true;
  boolIsInitialized = false;
}

/** Constructor
 *  @param[in] obj
 *  This geometry handler will be Object's geometry handler
 */
GeometryHandler::GeometryHandler(boost::shared_ptr<Object> obj)
    : Obj(obj.get()) {
  ObjComp = nullptr;
  boolTriangulated = false;
  boolIsInitialized = false;
}

/** Constructor
 *  @param[in] obj
 *  This geometry handler will be Object's geometry handler
 */
GeometryHandler::GeometryHandler(Object *obj)
    : Obj(obj) {
  ObjComp = nullptr;
  boolTriangulated = false;
  boolIsInitialized = false;
}

/// Destructor
GeometryHandler::~GeometryHandler() { ObjComp = nullptr; }
}
}
