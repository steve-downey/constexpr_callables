// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: the template argument is not one of the twelve supported function types
// CLAUSE: [func.wrap.copy.general]
#include <beman/constexpr_callables/copyable_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::copyable_function;

copyable_function<int> f; // must not compile
