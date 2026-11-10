// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: F is not a member pointer or a function pointer
// CLAUSE: [func.wrap.ref.deduct] function_ref(constant_wrapper<c, F>, T&&) Constraints
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
    object       o;
    function_ref f(std::cw<[](object&) { return 0; }>, o); // must not compile
}
