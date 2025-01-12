# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(GoogleTest)

add_executable(beman.constexpr_callables.tests.identity)
target_sources(beman.constexpr_callables.tests.identity PRIVATE identity.test.cpp)
target_link_libraries(
    beman.constexpr_callables.tests.identity
    PRIVATE beman::constexpr_callables GTest::gtest GTest::gtest_main
)

gtest_add_tests(beman.constexpr_callables.tests.identity "" AUTO)
