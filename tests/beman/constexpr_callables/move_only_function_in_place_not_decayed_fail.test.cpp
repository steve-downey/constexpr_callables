// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: VT is not the same type as T: T is const-qualified
// CLAUSE: [func.wrap.move.ctor] in_place ctor Mandates
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::move_only_function;

struct callable {
    int operator()() const { return 0; }
};
void test() {
    move_only_function<int()> f(std::in_place_type<const callable>); // must not compile
}
