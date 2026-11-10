// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: is_constructible_v<VT, Args...> is false
// CLAUSE: [func.wrap.copy.ctor] in_place ctor Constraints
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::copyable_function;

struct needs_int {
    explicit needs_int(int) {}
    int operator()() const { return 0; }
};
void test() {
    copyable_function<int()> f(std::in_place_type<needs_int>, "text"); // must not compile
}
