// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: the result of the call is not convertible to R
// CLAUSE: [func.wrap.copy.ctor] is-callable-from
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::copyable_function;

struct returns_int {
    int operator()() const { return 0; }
};
void test() {
    copyable_function<int*()> f(returns_int{}); // must not compile
}
