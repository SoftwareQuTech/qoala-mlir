add_mlir_translation_library(MLIRTargetiQoalaExport
        iQoalaContext.cpp
        ConvertToiQoala.cpp
        ModuleTranslation.cpp
        Module.cpp

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
        MLIRiQoalaMCLib
        QoalaAnalysisHelpers
        QoalaConversionHelpers
        MLIRTranslateLib
        MLIRTransformUtils
)

add_subdirectory(Dialect)
add_subdirectory(MC)