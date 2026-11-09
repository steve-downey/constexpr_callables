// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is-invocable-using<F> is false
// CLAUSE: [func.wrap.ref.ctor] function_ref(constant_wrapper<c, F>) Constraints
#include <beman/constexpr_callables/function_ref.hpp>

#include <utility>

using smd::constexpr_callables::function_ref;

struct object {
    int value = 0;
    int member(int x) { return value + x; }
};
int free_function(int x) { return x; }
int free_function_throws(int x) { return x; }

void test() {
    function_ref<int(object*)> f(std::cw<&free_function>); // must not compile
}
