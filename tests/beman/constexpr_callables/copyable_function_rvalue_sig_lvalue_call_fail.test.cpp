// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: ref is &&, so the call operator is not viable on an lvalue
// CLAUSE: [func.wrap.copy.inv] call operator qualifiers
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::copyable_function;

void test(copyable_function<int() &&>& f) {
    f(); // must not compile
}
