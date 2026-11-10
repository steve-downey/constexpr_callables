// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is_copy_constructible_v<VT> is false
// CLAUSE: [func.wrap.copy.ctor] in_place ctor Mandates
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::copyable_function;

struct move_only_callable {
    move_only_callable()                          = default;
    move_only_callable(move_only_callable&&)      = default;
    move_only_callable(const move_only_callable&) = delete;
    int operator()() const { return 0; }
};
void test() {
    copyable_function<int()> f(std::in_place_type<move_only_callable>); // must not compile
}
