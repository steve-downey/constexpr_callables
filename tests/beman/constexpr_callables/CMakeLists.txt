# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(GTest REQUIRED)

set(ALL_TESTS function_ref copyable_function move_only_function)

foreach(test ${ALL_TESTS})
    add_executable(beman.constexpr_callables.tests.${test})
    target_sources(
        beman.constexpr_callables.tests.${test}
        PRIVATE ${test}.test.cpp
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
