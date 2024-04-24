add_library(QoalaAnalysisHelpers
        STATIC
        Helpers.cpp
)

target_include_directories(QoalaAnalysisHelpers
        PRIVATE include/Analysis
)