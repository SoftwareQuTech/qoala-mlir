add_library(QoalaAnalysisHelpers
        STATIC
        Helpers.cpp
        ../../Target/iQoala/MC/NetQASMASM.cpp
)

target_include_directories(QoalaAnalysisHelpers
        PRIVATE include/Analysis
)