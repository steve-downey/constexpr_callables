// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: remove_cvref_t<F> is move_only_function, so the converting constructor is excluded and the copy
// constructor is deleted CLAUSE: [func.wrap.move.ctor] converting ctor Constraints
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::move_only_function;

void test(move_only_function<int()>& f) {
    move_only_function<int()> g(f); // must not compile
}
