// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: every argument type is a constexpr-param and
// constant_wrapper<INVOKE(f.value, remove_cvref_t<ArgTypes>::value...)> is valid
// CLAUSE: [func.wrap.ref.ctor] function_ref(constant_wrapper<c, F>) Mandates
#include <beman/constexpr_callables/function_ref.hpp>

#include <utility>

using beman::constexpr_callables::function_ref;

void test() {
    function_ref<int(decltype(std::cw<1>))> f(std::cw<[](int x) { return x + 1; }>); // must not compile
}
