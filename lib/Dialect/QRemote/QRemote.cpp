#include "Dialect/QRemote/QRemote.h"

using namespace mlir;
using namespace qoala::dialects::qremote;

// include generated source code for operations
#define GET_OP_CLASSES
#include "Dialect/QRemote/QRemote.cpp.inc"

// include generated source code for types
#define GET_TYPEDEF_CLASSES
#include "Dialect/QRemote/QRemoteTypes.cpp.inc"
