// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is_constructible_v<VT, initializer_list<U>&, Args...> is false
// CLAUSE: [func.wrap.copy.ctor] in_place ilist ctor Constraints
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::copyable_function;

struct no_ilist {
    int operator()() const { return 0; }
};
void test() {
    copyable_function<int()> f(std::in_place_type<no_ilist>, {1, 2, 3}); // must not compile
}
