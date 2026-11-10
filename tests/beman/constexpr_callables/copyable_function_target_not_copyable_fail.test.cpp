// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is_copy_constructible_v<VT> is false: a move-only callable passed as an rvalue
// CLAUSE: [func.wrap.copy.ctor] converting ctor Mandates
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::copyable_function;

void test() {
    auto                     move_only = [p = std::make_unique<int>(1)] { return *p; };
    copyable_function<int()> f(std::move(move_only)); // must not compile
}
