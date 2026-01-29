add_mlir_library(QoalaAnalysisHelpers
        Helpers.cpp
        PatternRewriteDriver.cpp

        LINK_LIBS PUBLIC
        MLIRIR
)

target_include_directories(QoalaAnalysisHelpers
        PRIVATE include/Analysis
)