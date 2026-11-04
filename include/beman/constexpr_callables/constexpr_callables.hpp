// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_CONSTEXPR_CALLABLES_CONSTEXPR_CALLABLES_HPP
#define BEMAN_CONSTEXPR_CALLABLES_CONSTEXPR_CALLABLES_HPP

#include <beman/constexpr_callables/config.hpp>

#if BEMAN_CONSTEXPR_CALLABLES_USE_MODULES() && !defined(BEMAN_CONSTEXPR_CALLABLES_INCLUDED_FROM_INTERFACE_UNIT)

import beman.constexpr_callables;

#else

    #include <beman/constexpr_callables/copyable_function.hpp>
    #include <beman/constexpr_callables/function_ref.hpp>
    #include <beman/constexpr_callables/move_only_function.hpp>

#endif // BEMAN_CONSTEXPR_CALLABLES_USE_MODULES() &&
       // !defined(BEMAN_CONSTEXPR_CALLABLES_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_CONSTEXPR_CALLABLES_CONSTEXPR_CALLABLES_HPP
