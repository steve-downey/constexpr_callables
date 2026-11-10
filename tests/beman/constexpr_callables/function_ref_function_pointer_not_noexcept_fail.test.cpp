// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: noex is true, so F must be nothrow-invocable
// CLAUSE: [func.wrap.ref.ctor] function_ref(F*) Constraints
#include <beman/constexpr_callables/function_ref.hpp>

using beman::constexpr_callables::function_ref;

struct object {
    int value = 0;
    int member(int x) { return value + x; }
};
int free_function(int x) { return x; }
int free_function_throws(int x) { return x; }

void test() {
    function_ref<int(int) noexcept> f(&free_function_throws); // must not compile
}
