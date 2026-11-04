export module beman.constexpr_callables;

import std;

#define BEMAN_CONSTEXPR_CALLABLES_INCLUDED_FROM_INTERFACE_UNIT
export {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <beman/constexpr_callables/constexpr_callables.hpp>
#pragma clang diagnostic pop
}
