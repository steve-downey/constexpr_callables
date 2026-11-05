// function_ref.hpp                                                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_CONSTEXPR_CALLABLES_FUNCTION_REF
#define INCLUDED_BEMAN_CONSTEXPR_CALLABLES_FUNCTION_REF

// Beyond [func.wrap.ref], the call operator is constexpr, so a function_ref
// bound to an object, or to a nontype_t entity, can be called during constant
// evaluation. Binding a function pointer stays runtime-only: the bound entity
// stores every function pointer as one common type, which takes a
// reinterpret_cast that is not allowed in a constant expression. That is why
// function_ref(F*) is not constexpr in the standard, and the same limit
// applies to function_ref(F&&) when F is a function type.

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace smd {
namespace constexpr_callables {

// [func.wrap.ref], non-owning wrapper
template <class... S>
class function_ref; // freestanding, not defined

template <auto V>
struct nontype_t {
    explicit nontype_t() = default;
};

template <auto V>
constexpr nontype_t<V> nontype{};

namespace detail {

// BoundEntityType: trivially copyable, holds a pointer to object or a pointer
// to function.
union bound_entity {
    const void* object;
    void (*function)();

    constexpr bound_entity() noexcept : object(nullptr) {}
};

template <class T>
inline constexpr bool is_nontype = false;

template <auto V>
inline constexpr bool is_nontype<nontype_t<V>> = true;

// `f != nullptr` for a pointer template argument, decided by template-argument
// equivalence rather than by evaluating a comparison: gcc cannot
// constant-evaluate a function address compared with null under the
// null-checking sanitizers, so the literal form rejects valid code there.
template <auto f>
inline constexpr bool is_null_entity =
    std::is_same_v<std::integral_constant<decltype(f), f>, std::integral_constant<decltype(f), nullptr>>;

template <auto f>
inline constexpr bool nontype_is_null = [] {
    using F = decltype(f);
    if constexpr (std::is_pointer_v<F> || std::is_member_pointer_v<F>) {
        return is_null_entity<f>;
    } else {
        return false;
    }
}();

template <bool Noex, class R, class... T>
inline constexpr bool invocable_using = Noex ? std::is_nothrow_invocable_r_v<R, T...> : std::is_invocable_r_v<R, T...>;

// [func.wrap.ref.deduct] for function_ref(nontype_t<f>, T&&).
template <class F, class T>
struct function_ref_object_signature {};

#define SMD_CONSTEXPR_CALLABLES_MEMBER_SIGNATURE(CV, REF)                        \
    template <class R, class G, class... A, bool E, class T>                     \
    struct function_ref_object_signature<R (G::*)(A...) CV REF noexcept(E), T> { \
        using type = R(A...) noexcept(E);                                        \
    }

SMD_CONSTEXPR_CALLABLES_MEMBER_SIGNATURE(, );
SMD_CONSTEXPR_CALLABLES_MEMBER_SIGNATURE(, &);
SMD_CONSTEXPR_CALLABLES_MEMBER_SIGNATURE(const, );
SMD_CONSTEXPR_CALLABLES_MEMBER_SIGNATURE(const, &);
SMD_CONSTEXPR_CALLABLES_MEMBER_SIGNATURE(volatile, );
SMD_CONSTEXPR_CALLABLES_MEMBER_SIGNATURE(volatile, &);
SMD_CONSTEXPR_CALLABLES_MEMBER_SIGNATURE(const volatile, );
SMD_CONSTEXPR_CALLABLES_MEMBER_SIGNATURE(const volatile, &);

#undef SMD_CONSTEXPR_CALLABLES_MEMBER_SIGNATURE

template <class M, class G, class T>
    requires std::is_object_v<M>
struct function_ref_object_signature<M G::*, T> {
    using type = std::invoke_result_t<M G::*, T&>();
};

template <class R, class G, class... A, bool E, class T>
struct function_ref_object_signature<R (*)(G, A...) noexcept(E), T> {
    using type = R(A...) noexcept(E);
};

} // namespace detail

/*
The header provides partial specializations of function_ref for each
combination of the possible replacements of the placeholders cv and noex
where:
  (1.1) cv is either const or empty, and
  (1.2) noex is either true or false.
*/
#define SMD_CONSTEXPR_CALLABLES_FUNCTION_REF_IMPL

#define SMD_CONSTEXPR_CALLABLES_CV
#include <beman/constexpr_callables/detail/function_ref_impl.hpp>
#undef SMD_CONSTEXPR_CALLABLES_CV

#define SMD_CONSTEXPR_CALLABLES_CV const
#include <beman/constexpr_callables/detail/function_ref_impl.hpp>
#undef SMD_CONSTEXPR_CALLABLES_CV

#undef SMD_CONSTEXPR_CALLABLES_FUNCTION_REF_IMPL

// [func.wrap.ref.deduct], deduction guides
template <class F>
    requires std::is_function_v<F>
function_ref(F*) -> function_ref<F>;

template <auto f>
    requires std::is_function_v<std::remove_pointer_t<decltype(f)>>
function_ref(nontype_t<f>) -> function_ref<std::remove_pointer_t<decltype(f)>>;

template <auto f, class T>
function_ref(nontype_t<f>, T&&) -> function_ref<typename detail::function_ref_object_signature<decltype(f), T>::type>;

} // namespace constexpr_callables
} // namespace smd

#endif
