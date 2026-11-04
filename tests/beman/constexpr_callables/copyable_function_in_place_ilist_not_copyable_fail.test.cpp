// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is_copy_constructible_v<VT> is false
// CLAUSE: [func.wrap.copy.ctor] in_place ilist ctor Mandates
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::copyable_function;

#include <initializer_list>
struct move_only_ilist_callable {
    move_only_ilist_callable(std::initializer_list<int>) {}
    move_only_ilist_callable(move_only_ilist_callable&&)      = default;
    move_only_ilist_callable(const move_only_ilist_callable&) = delete;
    int operator()() const { return 0; }
};
void test() {
    copyable_function<int()> f(std::in_place_type<move_only_ilist_callable>, {1, 2}); // must not compile
}
