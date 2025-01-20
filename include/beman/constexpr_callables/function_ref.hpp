// function_ref.hpp                                                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_CONSTEXPR_CALLABLES_FUNCTION_REF
#define INCLUDED_BEMAN_CONSTEXPR_CALLABLES_FUNCTION_REF

namespace smd {
namespace constexpr_callables {

// [func.wrap.ref], non-owning wrapper
template <class... S>
class function_ref; // freestanding, not defined
template <class R, class... ArgTypes>
class function_ref<R(ArgTypes...) noexcept(false)>; // freestanding, see below

template <auto V>
struct nontype_t {
    explicit nontype_t() = default;
};

template <auto V>
constexpr nontype_t<V> nontype{};
struct BoundEntityType {};

} // namespace constexpr_callables
} // namespace smd

#endif
