// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: the template argument is not one of the four supported function types
// CLAUSE: [func.wrap.ref.general]
#include <beman/constexpr_callables/function_ref.hpp>

using beman::constexpr_callables::function_ref;

struct object {
    int value = 0;
    int member(int x) { return value + x; }
};
int free_function(int x) { return x; }
int free_function_throws(int x) { return x; }

static_assert(sizeof(function_ref<int() &>) > 0); // must not compile
