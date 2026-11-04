// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: cv is const, so VT must be callable as const
// CLAUSE: [func.wrap.move.ctor] is-callable-from
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::move_only_function;

struct non_const_only {
    int operator()() { return 0; }
};
void test() {
    move_only_function<int() const> f(non_const_only{}); // must not compile
}
