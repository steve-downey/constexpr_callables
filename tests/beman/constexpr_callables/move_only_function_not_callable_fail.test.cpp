// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is-callable-from<VT> is false for a non-callable VT
// CLAUSE: [func.wrap.move.ctor] converting ctor Constraints
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::move_only_function;

void test() {
    move_only_function<int()> f(42); // must not compile
}
