add_mlir_translation_library(MLIRTargetiQoalaExport
        ConvertToiQoala.cpp
        ModuleTranslation.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/Target/iQoala

        DEPENDS
        intrinsics_gen

        LINK_COMPONENTS
        Core
        FrontendOpenMP
        TransformUtils
        TargetParser

        LINK_LIBS PUBLIC
        # TODO - Link the right dialect libraries!
        #MLIRDLTIDialect
        #MLIRLLVMDialect
        #MLIRLLVMIRTransforms
        MLIRTranslateLib
        MLIRTransformUtils
)

add_subdirectory(Dialect)