// move_only_function.hpp                                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_CONSTEXPR_CALLABLES_MOVE_ONLY_FUNCTION
#define INCLUDED_BEMAN_CONSTEXPR_CALLABLES_MOVE_ONLY_FUNCTION

namespace smd {
namespace constexpr_callables {

// [func.wrap.move], move-only wrapper

template <class... S>
class move_only_function; // not defined

// template <class R, class... ArgTypes>
// class move_only_function<R(ArgTypes...) cv ref noexcept(noex)>; // see below
template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) noexcept(false)>; // see below

template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) noexcept(true)>; // see below

template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) & noexcept(false)>; // see below

template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) & noexcept(true)>; // see below

template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) && noexcept(false)>; // see below

template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) && noexcept(true)>; // see below

template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) const noexcept(false)>; // see below

template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) const noexcept(true)>; // see below

template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) const & noexcept(false)>; // see below

template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) const & noexcept(true)>; // see below

template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) const && noexcept(false)>; // see below

template <class R, class... ArgTypes>
class move_only_function<R(ArgTypes...) const && noexcept(true)>; // see below

/*
The header provides partial specializations of move_only_function for each
combination of the possible replacements of the placeholders cv, ref, and noex
where
  (1.1) cv is either const or empty,
  (1.2) ref is either &, &&, or empty, and
  (1.3) noex is either true or false.

For each of the possible combinations of the placeholders mentioned above,
there is a placeholder inv-quals defined as follows:
  (2.1) If ref is empty, let inv-quals be cv&,
  (2.2) otherwise, let inv-quals be cv ref.
*/

} // namespace constexpr_callables
} // namespace smd

#endif
