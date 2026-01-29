add_library(QoalaAnalysisHelpers
        STATIC
        Helpers.cpp
        PatternRewriteDriver.cpp
)

target_include_directories(QoalaAnalysisHelpers
        PRIVATE include/Analysis
)