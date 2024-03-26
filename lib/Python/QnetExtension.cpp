#include "Qnet-C/Qnet-c.h"
#include "mlir/Bindings/Python/PybindAdaptors.h"

namespace py = pybind11;
using namespace mlir::python::adaptors;

static void registerQnetTypes(py::module &m) {
    /**
     * TODO - It seems that, for some reason, this type is not "exported"
     * correctly in the python bindings library. This means that we cannot
     * directly access the QubitType as a python class. However, we probably
     * don't really need to use this class, since the frontend will not create
     * new type instances using QubitType as a base (for an example of this, see
     * https://circt.llvm.org/docs/PythonBindings/#trying-things-out)
     */
    auto qubitType = mlir_type_subclass(m, "QubitType", mlirTypeIsAQubitType);
}

// This file will be compiled as the "_qnetTypes" python module, so we need
// to declare such module in the "name" argument of this macro
PYBIND11_MODULE(_qnetTypes, m) {
    // We declare a submodule called "qnet_types"
    auto qnetTypesM = m.def_submodule("qnet_types");

    // Registration of the Qnet dialect under the "qir_types" module
    qnetTypesM.def(
        // We define a "register_dialect" function under the "qir_types" module
        "register_dialect",
        [](MlirContext context, bool load) {
            MlirDialectHandle handle = mlirGetDialectHandle__qnet__();
            mlirDialectHandleRegisterDialect(handle, context);
            if (load) {
                mlirDialectHandleLoadDialect(handle, context);
            }
        },
        py::arg("context") = py::none(), py::arg("load") = true);
    registerQnetTypes(qnetTypesM);
}
