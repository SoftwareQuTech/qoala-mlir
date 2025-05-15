include(AddMLIRPython)
# We need to detect the MLIR python env, so we have access to the "pybind11"
# package, which was installed as part of the packages required by MLIR
include(MLIRDetectPythonEnv)
mlir_configure_python_dev_packages()

add_compile_definitions("MLIR_PYTHON_PACKAGE_PREFIX=qnet.")

declare_mlir_python_sources(QNetPythonSources)

# We declare the python bindings we want to compile
# This commands invokes `mlir-tblgen -gen-python-op-bindings -bind-dialect=<namespace> <td_file>`
# to create the python sources based on the given td file
# This cmake command also gives some extra options:
# * ROOT_DIR: defines the root folder for the files.
# * TO_FILE: counter-intuitively, this is the <td_file> we want to compile
# * SOURCES: this is an "extra" python file to include in the compiled python sources.
#            As per recommendation of the docs, this file should include all the symbols
#            generated in the _<namespace>_gen_ops.py file.
#            Since this is the file we will later use in the frontend, we cna also include
#            other defs and symbols in this "entry" python module.
declare_mlir_dialect_python_bindings(
        ADD_TO_PARENT QNetPythonSources
        ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mlir_qnet"
        TD_FILE dialects/QNetOps.td
        SOURCES
            dialects/qnet.py
        DIALECT_NAME qnet)

# At the moment, generating the python bindings for MLIR dialects _only_ works for the declared
# *operations*. Types need to be declared in a C++ class which is invoked from the python side
# before using the module. These "manually declared" types are included in a new python extension
# To check how the types need to be declared, see:
# https://github.com/llvm/circt/blob/218617cf1f6b1a5058bee178076ee9197429132e/lib/Bindings/Python/HWModule.cpp
declare_mlir_python_extension(QNetPythonSources.Extension
        MODULE_NAME _qnetTypes
        ADD_TO_PARENT QNetPythonSources
        SOURCES
        QNetExtension.cpp
        EMBED_CAPI_LINK_LIBS
            QNetCAPI
        PRIVATE_LINK_LIBS
            LLVMSupport
)

# Registration of the plugin. We need to link a python library, containing *ALL* of the
# MLIR libraries, which takes a long time and creates a python plugin of around 150MB.
# So far this is the best we can do, and there is still no way to do a more lightweight
# linking of the final python plugin.
add_mlir_python_common_capi_library(QNetPythonCAPI
        INSTALL_COMPONENT QNetPythonModules
        INSTALL_DESTINATION qnet/_mlir_libs
        OUTPUT_DIRECTORY "${MLIR_BINARY_DIR}/python_packages/qnet_bindings/qnet/_mlir_libs"
        RELATIVE_INSTALL_ROOT "../../../.."
        DECLARED_SOURCES
            QNetPythonSources.Extension
            MLIRPythonExtension.RegisterEverything
            MLIRPythonSources.Core
            # We no longer bring "everything" (all the dialects) in the linkage.
            # This means that compilation is faster and the generated library is lighter
            # This also means that we now need to "include" the mlir_core python bindings
            # to use the "common" dialects. This can be done by modifying the PYHTONPATH var
)

# Instantiation of the python extensions
add_mlir_python_modules(QNetPythonModules
        ROOT_PREFIX "${MLIR_BINARY_DIR}/python_packages/qnet_bindings/qnet"
        INSTALL_PREFIX "qnet"
        DECLARED_SOURCES
            # We no longer bring "everything" (all the dialects) in the linkage.
            # This means that compilation is faster and the generated library is lighter
            # This also means that we now need to "include" the mlir_core python bindings
            # to use the "common" dialects. This can be done by modifying the PYHTONPATH var
            QNetPythonSources.Extension
            QNetPythonSources
            MLIRPythonExtension.RegisterEverything
            MLIRPythonSources
        COMMON_CAPI_LINK_LIBS
            QNetPythonCAPI
)
