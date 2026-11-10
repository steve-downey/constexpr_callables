// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: ref is empty, so inv-quals is & and VT must be callable as an lvalue
// CLAUSE: [func.wrap.copy.ctor] is-callable-from
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::copyable_function;

struct rvalue_only {
    int operator()() && { return 0; }
};
void test() {
    copyable_function<int()> f(rvalue_only{}); // must not compile
}
