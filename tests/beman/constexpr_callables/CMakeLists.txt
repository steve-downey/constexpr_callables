# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(GoogleTest)

add_executable(beman.constexpr_callables.tests)
target_sources(beman.constexpr_callables.tests
  PRIVATE
  function_ref.test.cpp
  copyable_function.test.cpp
  move_only_function.test.cpp
)
target_link_libraries(
    beman.constexpr_callables.tests
    PRIVATE beman::constexpr_callables GTest::gtest GTest::gtest_main
)

gtest_add_tests(beman.constexpr_callables.tests "" AUTO)
