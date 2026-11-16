// function_ref.hpp                                                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_CONSTEXPR_CALLABLES_FUNCTION_REF
#define INCLUDED_BEMAN_CONSTEXPR_CALLABLES_FUNCTION_REF

// Beyond [func.wrap.ref], the call operator is constexpr, so a function_ref
// bound to an object, or to a constant_wrapper entity, can be called during
// constant evaluation. Binding a function pointer stays runtime-only: the bound entity
// stores every function pointer as one common type, which takes a
// reinterpret_cast that is not allowed in a constant expression. That is why
// function_ref(F*) is not constexpr in the standard, and the same limit
// applies to function_ref(F&&) when F is a function type.

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <version>

#if !defined(__cpp_lib_constant_wrapper)
    #error "function_ref requires std::constant_wrapper (__cpp_lib_constant_wrapper)"
#endif

namespace beman::constexpr_callables {

// [func.wrap.ref], non-owning wrapper
template <class... S>
class function_ref; // freestanding, not defined

namespace detail {

// BoundEntityType: trivially copyable, holds a pointer to object or a pointer
// to function.

//! \omit
union bound_entity {
    const void* object;
    void (*function)();

    constexpr bound_entity() noexcept : object(nullptr) {}
};

template <class T>
inline constexpr bool is_constant_wrapper = false;

template <auto c, class F>
inline constexpr bool is_constant_wrapper<std::constant_wrapper<c, F>> = true;

// `f != nullptr` for a pointer template argument, decided by template-argument
// equivalence rather than by evaluating a comparison: gcc cannot
// constant-evaluate a function address compared with null under the
// null-checking sanitizers, so the literal form rejects valid code there.
template <auto f>
inline constexpr bool is_null_entity =
    std::is_same_v<std::integral_constant<decltype(f), f>, std::integral_constant<decltype(f), nullptr>>;

// The Mandates of the constant_wrapper constructors: if F is a pointer or
// member pointer type, f.value != nullptr.
template <class CW, class F>
inline constexpr bool constant_wrapper_is_null = [] {
    if constexpr (std::is_pointer_v<F> || std::is_member_pointer_v<F>) {
        return is_null_entity<CW::value>;
    } else {
        return false;
    }
}();

// constexpr-param ([utility.syn]).
template <class T>
concept constexpr_param = requires { typename std::constant_wrapper<T::value>; };

template <class CW, class... A>
concept invoke_is_constant = requires { typename std::constant_wrapper<std::invoke(CW::value, A::value...)>; };

// The second Mandates element of function_ref(constant_wrapper<c, F>): true if
// ArgTypes is not empty, every remove_cvref_t<ArgTypes> satisfies
// constexpr-param, and constant_wrapper<INVOKE(f.value,
// remove_cvref_t<ArgTypes>::value...)> is a valid type.
template <class CW, class... ArgTypes>
inline constexpr bool constant_wrapper_call_is_constant = [] {
    if constexpr (sizeof...(ArgTypes) > 0 && (constexpr_param<std::remove_cvref_t<ArgTypes>> && ...)) {
        return invoke_is_constant<CW, std::remove_cvref_t<ArgTypes>...>;
    } else {
        return false;
    }
}();

// The parts of a function_ref specialization's signature that
// is-convertible-from-specialization compares.
//! \omit
template <class T>
struct function_ref_signature {
    static constexpr bool is_function_ref = false;
};

//! \omit
template <class R, class... A, bool N>
struct function_ref_signature<function_ref<R(A...) noexcept(N)>> {
    static constexpr bool is_function_ref = true;
    using function_type                   = R(A...);
    static constexpr bool is_const        = false;
    static constexpr bool noex            = N;
};

//! \omit
template <class R, class... A, bool N>
struct function_ref_signature<function_ref<R(A...) const noexcept(N)>> {
    static constexpr bool is_function_ref = true;
    using function_type                   = R(A...);
    static constexpr bool is_const        = true;
    static constexpr bool noex            = N;
};

// is-convertible-from-specialization<From> in function_ref<To-signature>:
// From is function_ref<R(Args...) cv2 noexcept(noex2)> for the same R and
// Args, and both
//   is_convertible_v<R(&)(Args...) noexcept(noex2), R(&)(Args...) noexcept(noex)>
//   is_convertible_v<int cv&, int cv2&>
// hold. The first is true unless only the target is noexcept, the second
// unless only the target is const.
template <class From, class To>
inline constexpr bool is_convertible_function_ref = [] {
    using from = function_ref_signature<From>;
    using to   = function_ref_signature<To>;
    if constexpr (from::is_function_ref && to::is_function_ref) {
        if constexpr (std::is_same_v<typename from::function_type, typename to::function_type>) {
            return (from::noex || !to::noex) && (from::is_const || !to::is_const);
        } else {
            return false;
        }
    } else {
        return false;
    }
}();

template <bool Noex, class R, class... T>
inline constexpr bool invocable_using = Noex ? std::is_nothrow_invocable_r_v<R, T...> : std::is_invocable_r_v<R, T...>;

// [func.wrap.ref.deduct] for function_ref(constant_wrapper<c, F>, T&&).
//! \omit
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

//! \omit
template <class M, class G, class T>
    requires std::is_object_v<M>
struct function_ref_object_signature<M G::*, T> {
    using type = std::invoke_result_t<M G::*, T&>() noexcept;
};

//! \omit
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

template <auto c, class F0>
    requires std::is_function_v<std::remove_pointer_t<F0>>
function_ref(std::constant_wrapper<c, F0>) -> function_ref<std::remove_pointer_t<F0>>;

template <auto c, class F, class T>
function_ref(std::constant_wrapper<c, F>, T&&)
    -> function_ref<typename detail::function_ref_object_signature<F, T>::type>;

// ===========================================================================
// [func.wrap.ref] wording
// ===========================================================================
// The `//!` blocks below are the source of the [func.wrap.ref] wording;
// papers/P3574/wording/ is generated from them by specgen. Both partial
// specializations are defined in detail/function_ref_impl.hpp, outside the
// document, so the synopsis and every item's declaration are authored here,
// detached, in the draft's order. Where a block says `\asdraft` its wording is
// the working draft's, which specgen reads from the draft's LaTeX
// (papers/P3574/wording/README.md). Beyond the draft, the call operator is
// constexpr.

// clang-format off
// The wording below must keep its line breaks: clang-format would reflow the
// long lines of the verbatim synopsis and of the item declarations.

// \rSec3[func.wrap.ref]{Non-owning wrapper}

// \rSec4[func.wrap.ref.general]{General}

//! \asdraft

// \rSec4[func.wrap.ref.class]{Class template \tcode{function_ref}}

//! \verbatim-synopsis
//! namespace std {
//!   template<class R, class... ArgTypes>
//!   class function_ref<R(ArgTypes...) @\cv{}@ noexcept(@\placeholder{noex}@)> {
//!   public:
//!     // \ref{func.wrap.ref.ctor}, constructors and assignment operators
//!     template<class F> function_ref(F*) noexcept;
//!     template<class F> constexpr function_ref(F&&) noexcept;
//!     template<auto c, class F> constexpr function_ref(constant_wrapper<c, F>) noexcept;
//!     template<auto c, class F, class U>
//!       constexpr function_ref(constant_wrapper<c, F>, U&&) noexcept;
//!     template<auto c, class F, class T>
//!       constexpr function_ref(constant_wrapper<c, F>, @\cv{}@ T*) noexcept;
//!
//!     constexpr function_ref(const function_ref&) noexcept = default;
//!     constexpr function_ref& operator=(const function_ref&) noexcept = default;
//!     template<class T> function_ref& operator=(T) = delete;
//!
//!     // \ref{func.wrap.ref.inv}, invocation
//!     constexpr R operator()(ArgTypes...) const noexcept(@\placeholder{noex}@);
//!
//!   private:
//!     template<class... T>
//!       static constexpr bool @\exposidnc{is-invocable-using}@ = @\seebelownc@;             // \expos
//!
//!     R (*@\exposidnc{thunk-ptr}@)(@\exposidnc{BoundEntityType}@, ArgTypes&&...) noexcept(@\placeholdernc{noex}@);      // \expos
//!     @\exposidnc{BoundEntityType}@ @\exposidnc{bound-entity}@;                                       // \expos
//!   };
//!
//!   // \ref{func.wrap.ref.deduct}, deduction guides
//!   template<class F>
//!     function_ref(F*) -> function_ref<F>;
//!   template<auto c, class F0>
//!     function_ref(constant_wrapper<c, F0>) -> function_ref<@\seebelow@>;
//!   template<auto c, class F, class T>
//!     function_ref(constant_wrapper<c, F>, T&&) -> function_ref<@\seebelow@>;
//! }

//! \asdraft

// \rSec4[func.wrap.ref.ctor]{Constructors and assignment operators}

//! \asdraft
//! \verbatim-itemdecl
//! template<class... T>
//!   static constexpr bool @\exposid{is-invocable-using}@ = @\seebelow@;

//! \asdraft
//! \verbatim-itemdecl
//! template<class F>
//!   static constexpr bool @\exposid{is-convertible-from-specialization}@ = @\seebelow@;

//! \asdraft
//! \verbatim-itemdecl
//! template<class F> function_ref(F* f) noexcept;

//! \asdraft
//! \verbatim-itemdecl
//! template<class F> constexpr function_ref(F&& f) noexcept;

//! \asdraft
//! \verbatim-itemdecl
//! template<auto c, class F> constexpr function_ref(constant_wrapper<c, F> f) noexcept;

//! \asdraft
//! \verbatim-itemdecl
//! template<auto c, class F, class U>
//!   constexpr function_ref(constant_wrapper<c, F> f, U&& obj) noexcept;

//! \asdraft
//! \verbatim-itemdecl
//! template<auto c, class F, class T>
//!   constexpr function_ref(constant_wrapper<c, F> f, @\cv{}@ T* obj) noexcept;

//! \asdraft
//! \verbatim-itemdecl
//! template<class T> function_ref& operator=(T) = delete;

// \rSec4[func.wrap.ref.inv]{Invocation}

//! \asdraft
//! \verbatim-itemdecl
//! constexpr R operator()(ArgTypes... args) const noexcept(@\placeholder{noex}@);

// \rSec4[func.wrap.ref.deduct]{Deduction guides}

//! \asdraft
//! \verbatim-itemdecl
//! template<class F>
//!   function_ref(F*) -> function_ref<F>;

//! \asdraft
//! \verbatim-itemdecl
//! template<auto c, class F0>
//!   function_ref(constant_wrapper<c, F0>) -> function_ref<@\seebelow@>;

//! \asdraft
//! \verbatim-itemdecl
//! template<auto c, class F, class T>
//!   function_ref(constant_wrapper<c, F>, T&&) -> function_ref<@\seebelow@>;

// clang-format on

} // namespace beman::constexpr_callables

#endif
