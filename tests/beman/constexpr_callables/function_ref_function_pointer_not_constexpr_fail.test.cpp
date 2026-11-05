// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: binding a function pointer is not usable in a constant expression
// CLAUSE: [func.wrap.ref.ctor] function_ref(F*), constexpr extension boundary
#include <beman/constexpr_callables/function_ref.hpp>

using smd::constexpr_callables::function_ref;
using smd::constexpr_callables::nontype;

struct object {
    int value = 0;
    int member(int x) { return value + x; }
};
int free_function(int x) { return x; }
int free_function_throws(int x) { return x; }

constexpr int twice(int x) { return 2 * x; }
constexpr int probe() {
    function_ref<int(int)> f(&twice);
    return f(1);
}
static_assert(probe() == 2); // must not compile
