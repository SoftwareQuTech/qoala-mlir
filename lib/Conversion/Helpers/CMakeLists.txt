add_library(QoalaConversionHelpers
        STATIC
        PrintHelpers.cpp
        AngleTransformHelper.cpp
)

target_include_directories(QoalaConversionHelpers
        PRIVATE include/Conversion
)