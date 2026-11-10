// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: the template argument is not one of the twelve supported function types
// CLAUSE: [func.wrap.move.general]
#include <beman/constexpr_callables/move_only_function.hpp>

#include <memory>
#include <utility>

using beman::constexpr_callables::move_only_function;

move_only_function<int> f; // must not compile
