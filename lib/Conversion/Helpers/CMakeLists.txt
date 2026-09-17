add_library(QoalaConversionHelpers
        STATIC
        PrintHelpers.cpp
        TypeConversionHelpers.cpp
        AngleTransformHelper.cpp
)

target_include_directories(QoalaConversionHelpers
        PRIVATE include/Conversion
)