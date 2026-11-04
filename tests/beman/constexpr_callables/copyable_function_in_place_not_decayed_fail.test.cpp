// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: VT is not the same type as T: T is const-qualified
// CLAUSE: [func.wrap.copy.ctor] in_place ctor Mandates
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using smd::constexpr_callables::copyable_function;

struct callable {
    int operator()() const { return 0; }
};
void test() {
    copyable_function<int()> f(std::in_place_type<const callable>); // must not compile
}
