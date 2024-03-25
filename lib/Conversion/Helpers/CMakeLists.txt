add_library(QoalaConversionHelpers
        STATIC
        Helpers.cpp
)

target_include_directories(QoalaConversionHelpers
        PRIVATE include/Conversion
)