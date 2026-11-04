// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: VT is not the same type as T: T is const-qualified
// CLAUSE: [func.wrap.move.ctor] in_place ilist ctor Mandates
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::move_only_function;

#include <initializer_list>
struct ilist_callable {
    ilist_callable(std::initializer_list<int>) {}
    int operator()() const { return 0; }
};
void test() {
    move_only_function<int()> f(std::in_place_type<const ilist_callable>, {1, 2, 3}); // must not compile
}
