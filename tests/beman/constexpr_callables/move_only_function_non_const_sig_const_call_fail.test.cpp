// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: cv is empty, so the call operator is not viable on a const object
// CLAUSE: [func.wrap.move.inv] call operator qualifiers
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::move_only_function;

void test(const move_only_function<int()>& f) {
    f(); // must not compile
}
