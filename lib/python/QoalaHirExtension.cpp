#include "qoalahir-c/qoalahir.h"
#include "mlir/Bindings/Python/PybindAdaptors.h"

namespace py = pybind11;
using namespace mlir::python::adaptors;

// This file will be compiled as the "_qoalaQirTypes" python module, so we need
// to declare such module in the name
PYBIND11_MODULE(_qoalaQirTypes, m) {
    // We declare a submodule called "qir_types"
    auto qirTypesM = m.def_submodule("qir_types");

    // Registration of the QoalaHir dialect under the "qir_types" module
    qirTypesM.def(
            // We define a "register_dialect" function under the "qir_types" module
            "register_dialect",
                [](MlirContext context, bool load) {
                MlirDialectHandle handle = mlirGetDialectHandle__hir__();
                mlirDialectHandleRegisterDialect(handle, context);
                if (load) {
                    mlirDialectHandleLoadDialect(handle, context);
                }
            },
            py::arg("context") = py::none(), py::arg("load") = true
    );

}
