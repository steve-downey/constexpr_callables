// tests/beman/constexpr_callables/copyable_function.test.cpp                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/constexpr_callables/copyable_function.hpp>

#include <beman/constexpr_callables/copyable_function.hpp> // test 2nd include OK

#include <gtest/gtest.h>

TEST(TestCopyableFunction, Fail) { SUCCEED(); }
