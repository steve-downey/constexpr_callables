// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: remove_cvref_t<F> is a specialization of in_place_type_t, so it is not a converting source
// CLAUSE: [func.wrap.copy.ctor] converting ctor Constraints
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::copyable_function;

struct callable {
    int operator()() const { return 0; }
};
void test() {
    copyable_function<int()> f = std::in_place_type<callable>; // must not compile
}
