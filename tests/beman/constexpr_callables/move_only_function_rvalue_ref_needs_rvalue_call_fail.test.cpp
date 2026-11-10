// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: ref is &&, so VT must be callable as an rvalue
// CLAUSE: [func.wrap.move.ctor] is-callable-from
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::move_only_function;

struct lvalue_only {
    int operator()() & { return 0; }
};
void test() {
    move_only_function<int() &&> f(lvalue_only{}); // must not compile
}
