// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/constexpr_callables/function_ref.hpp>

#include <functional>

int twice(int value) noexcept {
    return 2 * value;
}

int main() {
    beman::constexpr_callables::function_ref<int(int) noexcept> callable(std::cw<&twice>);
    return callable(21) == 42 ? 0 : 1;
}
