add_library(QoalaConversionHelpers
        STATIC
        PrintHelpers.cpp
)

target_include_directories(QoalaConversionHelpers
        PRIVATE include/Conversion
)