// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: ref is &&, so the call operator is not viable on an lvalue
// CLAUSE: [func.wrap.move.inv] call operator qualifiers
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::move_only_function;

void test(move_only_function<int() &&>& f) {
    f(); // must not compile
}
