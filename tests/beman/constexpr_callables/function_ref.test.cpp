// tests/beman/constexpr_callables/function_ref.test.cpp                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/constexpr_callables/function_ref.hpp>

#include <beman/constexpr_callables/function_ref.hpp> // test 2nd include OK

#include <gtest/gtest.h>

TEST(TestFunctionRef, Fail) { SUCCEED(); }
