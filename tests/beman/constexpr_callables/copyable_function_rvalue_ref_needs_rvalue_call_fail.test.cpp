// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: ref is &&, so VT must be callable as an rvalue
// CLAUSE: [func.wrap.copy.ctor] is-callable-from
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::copyable_function;

struct lvalue_only {
    int operator()() & { return 0; }
};
void test() {
    copyable_function<int() &&> f(lvalue_only{}); // must not compile
}
