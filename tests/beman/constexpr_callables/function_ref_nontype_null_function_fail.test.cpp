// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: f is a null function pointer
// CLAUSE: [func.wrap.ref.ctor] function_ref(nontype_t<f>) Mandates
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
    function_ref<int(int)> f(nontype<static_cast<int (*)(int)>(nullptr)>); // must not compile
}
