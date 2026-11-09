// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: cv is const, so f must be invocable with a const T&
// CLAUSE: [func.wrap.ref.ctor] function_ref(constant_wrapper<c, F>, U&&) Constraints
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
    object                       o;
    function_ref<int(int) const> f(std::cw<&object::member>, o); // must not compile
}
