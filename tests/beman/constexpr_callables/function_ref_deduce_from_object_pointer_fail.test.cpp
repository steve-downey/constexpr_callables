// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is_function_v<F> is false for a pointer to object
// CLAUSE: [func.wrap.ref.deduct] function_ref(F*) Constraints
#include <beman/constexpr_callables/function_ref.hpp>

using beman::constexpr_callables::function_ref;

struct object {
    int value = 0;
    int member(int x) { return value + x; }
};
int free_function(int x) { return x; }
int free_function_throws(int x) { return x; }

void test() {
    int          i = 0;
    function_ref f(&i); // must not compile
}
