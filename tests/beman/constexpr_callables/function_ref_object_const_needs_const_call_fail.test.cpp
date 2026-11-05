// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: cv is const, so T must be callable as const T&
// CLAUSE: [func.wrap.ref.ctor] function_ref(F&&) Constraints
#include <beman/constexpr_callables/function_ref.hpp>

using smd::constexpr_callables::function_ref;
using smd::constexpr_callables::nontype;

struct object {
    int value = 0;
    int member(int x) { return value + x; }
};
int free_function(int x) { return x; }
int free_function_throws(int x) { return x; }

struct non_const_only {
    int operator()() { return 0; }
};
void test() {
    non_const_only            n;
    function_ref<int() const> f(n); // must not compile
}
