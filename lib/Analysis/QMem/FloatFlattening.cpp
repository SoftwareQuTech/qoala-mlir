#include "Analysis/QMem/Conversion.h"

namespace qoala::analysis::flattening {
    void flattenFloatInstances(mlir::ModuleOp &module) {
        // TODO - Create an analysis that identify all operations that return an f32
        //  and insert a call to the qoala builtin to "flatten" it to 2xi32
    }
}