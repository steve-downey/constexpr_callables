// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is-callable-from<VT> is false
// CLAUSE: [func.wrap.move.ctor] in_place ctor Constraints
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::move_only_function;

struct not_callable {};
void test() {
    move_only_function<int()> f(std::in_place_type<not_callable>); // must not compile
}
