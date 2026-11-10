// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: cv is empty, so the call operator is not viable on a const object
// CLAUSE: [func.wrap.copy.inv] call operator qualifiers
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::copyable_function;

void test(const copyable_function<int()>& f) {
    f(); // must not compile
}
