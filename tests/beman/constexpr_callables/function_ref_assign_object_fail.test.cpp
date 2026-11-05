// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: assigning a callable object, which is not function_ref, a pointer, or a nontype_t
// CLAUSE: [func.wrap.ref.ctor] operator=(T) deleted
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
    auto                   one = [](int x) { return x; };
    auto                   two = [](int x) { return x + 1; };
    function_ref<int(int)> f   = one;
    f                          = two; // must not compile
}
