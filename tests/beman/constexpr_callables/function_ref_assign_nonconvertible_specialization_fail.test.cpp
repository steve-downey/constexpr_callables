// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: assigning a function_ref specialization for which
// is-convertible-from-specialization is false (it would drop const)
// CLAUSE: [func.wrap.ref.ctor] operator=(T) deleted
#include <beman/constexpr_callables/function_ref.hpp>

using smd::constexpr_callables::function_ref;

void test() {
    auto                      l = [] { return 0; };
    function_ref<int() const> c = l;
    function_ref<int()>       m = l;
    c                           = m; // must not compile
}
