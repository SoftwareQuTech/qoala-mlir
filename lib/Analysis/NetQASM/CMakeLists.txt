add_mlir_dialect_library(MLIRNetQASMAnalysis
        QubitAnalysis.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/NetQASM

#        LINK_LIBS PRIVATE
#        MLIRNetQASM
#
#        DEPENDS
#        MLIRNetQASM
#        MLIRNetQASMInterfacesIncGen
)
