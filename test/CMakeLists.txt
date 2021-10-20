configure_lit_site_cfg(
        ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
        ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
        MAIN_CONFIG
        ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
)

set(HIR_TEST_DEPENDS
        FileCheck count not
        hir-opt
        )

add_lit_testsuite(check-hir "Running the HIR regression tests"
        ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${HIR_TEST_DEPENDS}
        )
set_target_properties(check-hir PROPERTIES FOLDER "Tests")

add_lit_testsuites(HIR ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS ${HIR_TEST_DEPENDS})
