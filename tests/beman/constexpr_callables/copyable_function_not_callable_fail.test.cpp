// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is-callable-from<VT> is false for a non-callable VT
// CLAUSE: [func.wrap.copy.ctor] converting ctor Constraints
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::copyable_function;

void test() {
    copyable_function<int()> f(42); // must not compile
}
