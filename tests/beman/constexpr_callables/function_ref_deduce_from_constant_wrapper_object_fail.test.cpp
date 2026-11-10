// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is_function_v<remove_pointer_t<decltype(f)>> is false for a lambda
// CLAUSE: [func.wrap.ref.deduct] function_ref(constant_wrapper<c, F>) Constraints
#include <beman/constexpr_callables/function_ref.hpp>

#include <utility>

using beman::constexpr_callables::function_ref;

struct object {
    int value = 0;
    int member(int x) { return value + x; }
};
int free_function(int x) { return x; }
int free_function_throws(int x) { return x; }

void test() {
    function_ref f(std::cw<[](int x) { return x; }>); // must not compile
}
