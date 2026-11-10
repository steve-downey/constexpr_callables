// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: noex is true, so VT must be nothrow-invocable
// CLAUSE: [func.wrap.copy.ctor] is-callable-from
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::copyable_function;

struct may_throw {
    int operator()() { return 0; }
};
void test() {
    copyable_function<int() noexcept> f(may_throw{}); // must not compile
}
