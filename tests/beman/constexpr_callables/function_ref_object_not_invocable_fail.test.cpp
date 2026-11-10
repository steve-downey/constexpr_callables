// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is-invocable-using<cv T&> is false
// CLAUSE: [func.wrap.ref.ctor] function_ref(F&&) Constraints
#include <beman/constexpr_callables/function_ref.hpp>

using beman::constexpr_callables::function_ref;

struct object {
    int value = 0;
    int member(int x) { return value + x; }
};
int free_function(int x) { return x; }
int free_function_throws(int x) { return x; }

void test() {
    object              o;
    function_ref<int()> f(o); // must not compile
}
