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
// document, so the synopsis and every item are authored here, detached, in
// the draft's order. Beyond the draft, the call operator is constexpr.

// clang-format off
// The wording below must keep its line breaks: clang-format would reflow long
// lines, start some with a LaTeX macro (which specgen #253 takes for a tag),
// and split lines of the verbatim synopsis.

// \rSec3[func.wrap.ref]{Non-owning wrapper}

// \rSec4[func.wrap.ref.general]{General}

//! The header provides partial specializations of `function_ref`
//! for each combination of the possible replacements of
//! the placeholders \cv{} and \placeholder{noex} where:
//! \item \cv{} is either const or empty, and
//! \item \placeholder{noex} is either `true` or `false`.

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
//!     R operator()(ArgTypes...) const noexcept(@\placeholder{noex}@);
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

// No line of the prose below begins with a LaTeX macro: specgen would take it
// for a tag (specgen #253), so each wraps after the first word of a macro's
// line instead.

//! An object of class \tcode{function_ref<R(Args...) \cv{} noexcept(\placeholder{noex})>}
//! stores a pointer to function \exposid{thunk-ptr} and
//! an object \exposid{bound-entity}.
//! The object \exposid{bound-entity} has
//! an unspecified trivially copyable type \exposid{BoundEntityType}, that
//! models \libconcept{copyable} and
//! is capable of storing a pointer to object value or a pointer to function value.
//! The type of \exposid{thunk-ptr} is \tcode{R(*)(\exposidnc{BoundEntityType}, Args\&\&...) noexcept(\placeholder{noex})}.
//!
//! Each specialization of `function_ref` is
//! a trivially copyable type\iref{term.trivially.copyable.type}
//! that models \libconcept{copyable}.
//!
//! Within subclause~\ref{func.wrap.ref}, \tcode{\placeholder{call-args}} is an argument pack with elements such that
//! \codeblock
//! decltype((@\placeholder{call-args}@))...
//! \endcodeblock
//! denote `ArgTypes&&...` respectively.

// \rSec4[func.wrap.ref.ctor]{Constructors and assignment operators}

//! If \placeholder{noex} is `true`, \tcode{\exposid{is-invocable-using}<T...>} is equal to:
//! \codeblock
//! is_nothrow_invocable_r_v<R, T..., ArgTypes...>
//! \endcodeblock
//! Otherwise, \tcode{\exposid{is-invocable-using}<T...>} is equal to:
//! \codeblock
//! is_invocable_r_v<R, T..., ArgTypes...>
//! \endcodeblock
//! \verbatim-itemdecl
//! template<class... T>
//!   static constexpr bool @\exposid{is-invocable-using}@ = @\seebelow@;

//! If `F` denotes a specialization \tcode{function_ref<R(Args...) \cvqual{cv2} noexcept(\placeholder{noex2})>}
//! for some placeholders \cvqual{cv2} and \placeholder{noex2}, \tcode{\exposid{is-convertible-from-specialization}<F>}
//! is equal to:
//! \codeblock
//! is_convertible_v<R(&)(Args...) noexcept(@\placeholder{noex2}@), R(&)(Args...) noexcept(@\placeholder{noex}@)> &&
//! is_convertible_v<int @\cv{}@&, int @\cvqual{cv2}@&>.
//! \endcodeblock
//! Otherwise, \tcode{\exposid{is-convertible-from-specialization}<F>} is `false`.
//! \verbatim-itemdecl
//! template<class F>
//!   static constexpr bool @\exposid{is-convertible-from-specialization}@ = @\seebelow@;

//! \constraints
//! \item `is_function_v<F>` is `true`, and
//! \item \tcode{\exposid{is-invocable-using}<F>} is `true`.
//! \expects `f` is not a null pointer.
//! \effects Initializes \exposid{bound-entity} with `f`, and \exposid{thunk-ptr}
//! with the address of a function \tcode{\placeholder{thunk}}
//! such that \tcode{\placeholder{thunk}(\exposid{bound-entity}, \placeholder{call-args}...)}
//! is expression-equivalent\iref{defns.expression.equivalent}
//! to \tcode{invoke_r<R>(f, \placeholder{call-args}...)}.
//! \verbatim-itemdecl
//! template<class F> function_ref(F* f) noexcept;

//! Let `T` be `remove_reference_t<F>`.
//! \constraints
//! \item `remove_cvref_t<F>` is not the same type as `function_ref`,
//! \item `is_member_pointer_v<T>` is `false`, and
//! \item \tcode{\exposid{is-invocable-using}<\cv{} T\&>} is `true`.
//! \effects If \tcode{\exposid{is-convertible-from-specialization}<remove_cv_t<T>>}
//! is `false`, initializes \exposid{bound-entity} with `addressof(f)`, and \exposid{thunk-ptr}
//! with the address of a function \tcode{\placeholder{thunk}}
//! such that \tcode{\placeholder{thunk}(\exposid{bound-entity}, \placeholder{call-args}...)}
//! is expression-equivalent\iref{defns.expression.equivalent}
//! to \tcode{invoke_r<R>(static_cast<\cv{} T\&>(f), \placeholder{call-args}...)}.
//! Otherwise, initializes \exposid{bound-entity}
//! with the value of \tcode{f.\exposid{bound-entity}} and \exposid{thunk-ptr}
//! with the value of \tcode{f.\exposid{thunk-ptr}}.
//! \remarks If `remove_cvref_t<F>` is a specialization of `function_ref`,
//! an implementation may initialize \exposid{bound-entity}
//! with the value of \tcode{f.\exposid{bound-entity}} and \exposid{thunk-ptr}
//! with the value of \tcode{f.\exposid{thunk-ptr}}.
//! \example
//! \codeblock
//! void f1(std::string);
//! void f2(std::string);
//!
//! function_ref<void(std::string)> r1(&f1);
//! function_ref<void(std::string&&)> r2(r1);
//! r2("");                 // \tcode{f1} is invoked
//! r1 = &f2;
//! r2("");                 // it is unspecified if \tcode{f1} or \tcode{f2} is invoked
//! \endcodeblock
//! \endexample
//! \verbatim-itemdecl
//! template<class F> constexpr function_ref(F&& f) noexcept;

//! \constraints \tcode{\exposid{is-invocable-using}<const F\&>} is `true`.
//! \mandates
//! \item If `is_pointer_v<F> || is_member_pointer_v<F>` is `true`,
//! then `f.value != nullptr` is `true`, and
//! \item if `ArgTypes` is not an empty pack and
//! all types in `remove_cvref_t<ArgTypes>...`
//! satisfy \exposconcept{constexpr-param} then \tcode{constant_wrapper<\placeholdernc{INVOKE}(f.value, remove_cvref_t<ArgTypes>::\newline value...)>}
//! is not a valid type.
//! \effects Initializes \exposid{bound-entity} with a pointer to an unspecified object or
//! null pointer value, and \exposid{thunk-ptr} with the address of a function \tcode{\placeholder{thunk}}
//! such that \tcode{\placeholder{thunk}(\exposid{bound-entity}, \placeholder{call-args}...)}
//! is expression-equivalent\iref{defns.expression.equivalent}
//! to \tcode{invoke_r<R>(f.value, \placeholder{call-args}...)}.
//! \verbatim-itemdecl
//! template<auto c, class F> constexpr function_ref(constant_wrapper<c, F> f) noexcept;

//! Let `T` be `remove_reference_t<U>`.
//! \constraints
//! \item `is_rvalue_reference_v<U&&>` is `false`, and
//! \item \tcode{\exposid{is-invocable-using}<const F\&, \cv{} T\&>} is `true`.
//! \mandates If `is_pointer_v<F> || is_member_pointer_v<F>` is `true`,
//! then `f.value != nullptr` is `true`.
//! \effects Initializes \exposid{bound-entity} with `addressof(obj)`, and \exposid{thunk-ptr}
//! with the address of a function \tcode{\placeholder{thunk}}
//! such that \tcode{\placeholder{thunk}(\exposid{bound-entity}, \placeholder{call-args}...)}
//! is expression-equivalent\iref{defns.expression.equivalent}
//! to \tcode{invoke_r<R>(f.value, static_cast<\cv{} T\&>(obj), \placeholder{call-args}...)}.
//! \verbatim-itemdecl
//! template<auto c, class F, class U>
//!   constexpr function_ref(constant_wrapper<c, F> f, U&& obj) noexcept;

//! \constraints \tcode{\exposid{is-invocable-using}<const F\&, \cv{} T*>} is `true`.
//! \mandates If `is_pointer_v<F> || is_member_pointer_v<F>` is `true`,
//! then `f.value != nullptr` is `true`.
//! \expects If `is_member_pointer_v<F>` is `true`,
//! `obj` is not a null pointer.
//! \effects Initializes \exposid{bound-entity} with `obj`, and \exposid{thunk-ptr}
//! with the address of a function \tcode{\placeholder{thunk}}
//! such that \tcode{\placeholder{thunk}(\exposid{bound-entity}, \placeholder{call-args}...)}
//! is expression-equivalent\iref{defns.expression.equivalent}
//! to \tcode{invoke_r<R>(f.value, obj, \placeholder{call-args}...)}.
//! \verbatim-itemdecl
//! template<auto c, class F, class T>
//!   constexpr function_ref(constant_wrapper<c, F> f, @\cv{}@ T* obj) noexcept;

//! \constraints
//! \item \tcode{\exposid{is-convertible-from-specialization}<T>} is `false`,
//! \item `is_pointer_v<T>` is `false`, and
//! \item `T` is not a specialization of `constant_wrapper`.
//! \verbatim-itemdecl
//! template<class T> function_ref& operator=(T) = delete;

// \rSec4[func.wrap.ref.inv]{Invocation}

//! \effects Equivalent to: \tcode{return \exposid{thunk-ptr}(\exposid{bound-entity}, std::forward<ArgTypes>(args)...);}
//! \verbatim-itemdecl
//! R operator()(ArgTypes... args) const noexcept(@\placeholder{noex}@);

// \rSec4[func.wrap.ref.deduct]{Deduction guides}

//! \constraints `is_function_v<F>` is `true`.
//! \verbatim-itemdecl
//! template<class F>
//!   function_ref(F*) -> function_ref<F>;

//! Let `F` be `remove_pointer_t<F0>`.
//! \constraints `is_function_v<F>` is `true`.
//! \remarks The deduced type is `function_ref<F>`.
//! \verbatim-itemdecl
//! template<auto c, class F0>
//!   function_ref(constant_wrapper<c, F0>) -> function_ref<@\seebelow@>;

//! \constraints
//! \item `F` is of the form \tcode{R(G::*)(A...) \cv{} \opt{\&} noexcept(E)} for a type `G`, or
//! \item `F` is of the form
//! `M G::*` for a type `G` and an object type `M`,
//! in which case
//! let `R` be `invoke_result_t<F, T&>`,
//! `A...` be an empty pack, and
//! `E` be `true`, or
//! \item `F` is of the form
//! `R(*)(G, A...) noexcept(E)` for a type `G`.
//! \remarks The deduced type is `function_ref<R(A...) noexcept(E)>`.
//! \verbatim-itemdecl
//! template<auto c, class F, class T>
//!   function_ref(constant_wrapper<c, F>, T&&) -> function_ref<@\seebelow@>;

// clang-format on

} // namespace beman::constexpr_callables

#endif
