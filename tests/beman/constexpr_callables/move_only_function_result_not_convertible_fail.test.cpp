// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: the result of the call is not convertible to R
// CLAUSE: [func.wrap.move.ctor] is-callable-from
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::move_only_function;

struct returns_int {
    int operator()() const { return 0; }
};
void test() {
    move_only_function<int*()> f(returns_int{}); // must not compile
}
