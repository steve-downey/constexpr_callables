// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is_member_pointer_v<T> is true
// CLAUSE: [func.wrap.ref.ctor] function_ref(F&&) Constraints
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
    auto                      pmf = &object::member;
    function_ref<int(object&, int)> f(pmf); // must not compile
}
