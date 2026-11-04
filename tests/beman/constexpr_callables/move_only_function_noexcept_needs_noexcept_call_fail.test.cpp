// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: noex is true, so VT must be nothrow-invocable
// CLAUSE: [func.wrap.move.ctor] is-callable-from
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::move_only_function;

struct may_throw {
    int operator()() { return 0; }
};
void test() {
    move_only_function<int() noexcept> f(may_throw{}); // must not compile
}
