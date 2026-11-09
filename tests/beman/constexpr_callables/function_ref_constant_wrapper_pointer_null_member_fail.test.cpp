// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: f.value is a null member pointer
// CLAUSE: [func.wrap.ref.ctor] function_ref(constant_wrapper<c, F>, cv T*) Mandates
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
    object                 o;
    function_ref<int(int)> f(std::cw<static_cast<int (object::*)(int)>(nullptr)>, &o); // must not compile
}
