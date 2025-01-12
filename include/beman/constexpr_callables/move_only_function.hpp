// move_only_function.hpp                                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_CONSTEXPR_CALLABLES_MOVE_ONLY_FUNCTION
#define INCLUDED_BEMAN_CONSTEXPR_CALLABLES_MOVE_ONLY_FUNCTION

namespace smd {
namespace constexpr_callables {

// [func.wrap.copy], copyable wrapper
template <class... S>
class copyable_function; // not defined
template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) /*cv*/ /*ref*/ noexcept(/*noex*/ false)>; // see below

}
}

#endif
