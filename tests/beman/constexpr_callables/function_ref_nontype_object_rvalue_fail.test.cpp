// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is_rvalue_reference_v<U&&> is true: binding an rvalue object would dangle
// CLAUSE: [func.wrap.ref.ctor] function_ref(nontype_t<f>, U&&) Constraints
#include <beman/constexpr_callables/function_ref.hpp>

using smd::constexpr_callables::function_ref;
using smd::constexpr_callables::nontype;

struct object {
    int value = 0;
    int member(int x) { return value + x; }
};
int free_function(int x) { return x; }
int free_function_throws(int x) { return x; }

void test() {
    function_ref<int(int)> f(nontype<&object::member>, object{}); // must not compile
}
