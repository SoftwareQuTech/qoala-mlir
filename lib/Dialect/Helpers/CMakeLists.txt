add_library(QoalaDialectHelpers
        STATIC
        DialectHelpers.cpp
)

target_include_directories(QoalaDialectHelpers
        PRIVATE include/Dialect/Helpers
)