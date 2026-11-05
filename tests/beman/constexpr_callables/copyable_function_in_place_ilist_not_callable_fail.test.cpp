// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is-callable-from<VT> is false
// CLAUSE: [func.wrap.copy.ctor] in_place ilist ctor Constraints
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::copyable_function;

#include <initializer_list>
struct ilist_not_callable {
    ilist_not_callable(std::initializer_list<int>) {}
};
void test() {
    copyable_function<int()> f(std::in_place_type<ilist_not_callable>, {1, 2, 3}); // must not compile
}
