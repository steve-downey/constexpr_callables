// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is_copy_constructible_v<VT> is false: VT is a move_only_function
// CLAUSE: [func.wrap.copy.ctor] converting ctor Mandates
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::copyable_function;

#include <beman/constexpr_callables/move_only_function.hpp>
void test(smd::constexpr_callables::move_only_function<int()>&& m) {
    copyable_function<int()> f(std::move(m)); // must not compile
}
