// function_ref.hpp                                                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_CONSTEXPR_CALLABLES_FUNCTION_REF
#define INCLUDED_BEMAN_CONSTEXPR_CALLABLES_FUNCTION_REF

namespace smd {
namespace constexpr_callables {

template <class Sig>
class function_ref; // freestanding

template <auto V>
struct nontype_t {
    explicit nontype_t() = default;
};

template <auto V>
constexpr nontype_t<V> nontype{};
struct BoundEntityType {};
/*
  The header provides partial specializations of function_ref for each
  combination of the possible replacements of the placeholders cv and noex
  where:

  (1.1) cv is either const or empty, and
  (1.2) noex is either true or false.
*/
/*
template <class R, class... ArgTypes>
class function_ref<R(ArgTypes...) const noexcept(false)> {
  public:
    // [func.wrap.ref.ctor], constructors and assignment operators
    template <class F>
    function_ref(F*) noexcept;
    template <class F>
    constexpr function_ref(F&&) noexcept;
    template <auto f>
    constexpr function_ref(nontype_t<f>) noexcept;
    template <auto f, class U>
    constexpr function_ref(nontype_t<f>, U&&) noexcept;
    template <auto f, class T>
    constexpr function_ref(nontype_t<f>, cv T*) noexcept;

    constexpr function_ref(const function_ref&) noexcept            = default;
    constexpr function_ref& operator=(const function_ref&) noexcept = default;
    template <class T>
    function_ref& operator=(T) = delete;

    // [func.wrap.ref.inv], invocation
    R operator()(ArgTypes...) const noexcept(noex);

  private:
    template <class... T>
        static constexpr bool is_invocable_using; // exposition only

    R (*thunk_ptr)(BoundEntityType, Args&&...) noexcept(noex); // exposition only
    BoundEntityType bound_entity;                              // exposition only
};

// [func.wrap.ref.deduct], deduction guides
template <class F>
function_ref(F*) -> function_ref<F>;
template <auto f>
function_ref(nontype_t<f>) -> function_ref<see below>;
template <auto f, class T>
function_ref(nontype_t<f>, T&&) -> function_ref<see below>;
*/
} // namespace constexpr_callables
} // namespace smd

#endif
