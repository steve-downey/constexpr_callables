// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/constexpr_callables/identity.hpp>

#include <iostream>

namespace exe = beman::constexpr_callables;

int main() {
    std::cout << exe::identity()(2024) << '\n';
    return 0;
}
