# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(GTest REQUIRED)

set(ALL_TESTS function_ref copyable_function move_only_function)

foreach(test ${ALL_TESTS})
    add_executable(beman.constexpr_callables.tests.${test})
    target_sources(
        beman.constexpr_callables.tests.${test}
        PRIVATE ${test}.test.cpp
    )
    target_include_directories(
        beman.constexpr_callables.tests.${test}
        PRIVATE ${PROJECT_SOURCE_DIR}/tests
    )
    target_link_libraries(
        beman.constexpr_callables.tests.${test}
        PRIVATE beman::constexpr_callables GTest::gtest_main
    )
    if(BEMAN_CONSTEXPR_CALLABLES_USE_MODULES)
        set_target_properties(
            beman.constexpr_callables.tests.${test}
            PROPERTIES CXX_MODULE_STD ON
        )
    endif()
endforeach()

include(GoogleTest)
foreach(test ${ALL_TESTS})
    gtest_discover_tests(
        beman.constexpr_callables.tests.${test}
        DISCOVERY_TIMEOUT 60
    )
endforeach()

# =============================================================================
# Negative compile tests
# =============================================================================
# Each source must fail to compile, and the compiler output must match
# `reason`, a regex confirming the failure is the intended Constraints or
# Mandates violation rather than an unrelated build error. The object library
# is excluded from the default build; the ctest test builds it on demand.
#
# Usage:
#   add_fail_test(<name> <source> <reason>)
function(add_fail_test name source reason)
    set(target beman.constexpr_callables.tests.${name})
    add_library(${target} OBJECT)
    target_sources(${target} PRIVATE ${source})
    target_link_libraries(${target} PRIVATE beman::constexpr_callables)
    set_target_properties(
        ${target}
        PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
    )
    add_test(
        NAME ${name}
        COMMAND
            ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target ${target}
            --config $<CONFIGURATION>
    )
    set_tests_properties(${name} PROPERTIES PASS_REGULAR_EXPRESSION "${reason}")
endfunction()

include(${CMAKE_CURRENT_SOURCE_DIR}/move_only_function_fail.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/copyable_function_fail.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/function_ref_fail.cmake)
