add_library(QoalaConversionHelpers
        STATIC
        TypeConversionHelpers.cpp
        AngleTransformHelper.cpp
)

target_include_directories(QoalaConversionHelpers
        PRIVATE include/Conversion
)