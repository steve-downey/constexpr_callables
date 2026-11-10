// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is-invocable-using<F, cv T&> is false
// CLAUSE: [func.wrap.ref.ctor] function_ref(constant_wrapper<c, F>, U&&) Constraints
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
    object                     o;
    function_ref<int(object*)> f(std::cw<&object::member>, o); // must not compile
}
