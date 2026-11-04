// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is-callable-from<VT> is false
// CLAUSE: [func.wrap.move.ctor] in_place ilist ctor Constraints
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::move_only_function;

#include <initializer_list>
struct ilist_not_callable {
    ilist_not_callable(std::initializer_list<int>) {}
};
void test() {
    move_only_function<int()> f(std::in_place_type<ilist_not_callable>, {1, 2, 3}); // must not compile
}
