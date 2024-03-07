#include "qoalahir-c/qoalahir.h"
#include "mlir/Bindings/Python/PybindAdaptors.h"

namespace py = pybind11;
using namespace mlir::python::adaptors;

static void registerQoalaHirTypes(py::module &m) {
    /**
     * TODO - It seems that, for some reason, this type is not "exported" correctly in the
     *  python bindings library. This means that we cannot directly access the QubitType as
     *  a python class. However, we probably don't really need to use this class, since
     *  the frontend will not create new type instances using QubitType as a base
     *  (for an example of this, see https://circt.llvm.org/docs/PythonBindings/#trying-things-out)
     */
    auto qubitType = mlir_type_subclass(m, "QubitType", mlirTypeIsAQubitType);

}

// This file will be compiled as the "_qoalaQirTypes" python module, so we need
// to declare such module in the "name" argument of this macro
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
    registerQoalaHirTypes(qirTypesM);
}
