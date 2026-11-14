// move_only_function.hpp                                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_CONSTEXPR_CALLABLES_MOVE_ONLY_FUNCTION
#define INCLUDED_BEMAN_CONSTEXPR_CALLABLES_MOVE_ONLY_FUNCTION

// The `//!` docblocks in this header are the source of the [func.wrap.move]
// wording; papers/P3574/wording/ is generated from them by specgen. Where one
// says `\asdraft` its wording is the working draft's, which specgen reads from
// the draft's LaTeX (papers/P3574/wording/README.md). All twelve
// partial specializations the wording describes are this one partial
// specialization: the parts that vary with cv, ref, and noex (the call
// operator and is-callable-from) come from detail::call_base<..., Sig>, which
// is defined only for those twelve forms of Sig. The synopsis is therefore
// authored in the draft's placeholder form rather than extracted.

#include <beman/constexpr_callables/detail/call_base.hpp>

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace beman::constexpr_callables {

template <class... S>
class move_only_function; // not defined

// \rSec3[func.wrap.move]{Move-only wrapper}

// \rSec4[func.wrap.move.general]{General}

//! \asdraft

// \rSec4[func.wrap.move.class]{Class template \tcode{move_only_function}}

//! \asdraft
//! \verbatim-synopsis
//! template<class R, class... ArgTypes>
//! class move_only_function<R(ArgTypes...) @\cv{}@ @\placeholder{ref}@ noexcept(@\placeholder{noex}@)> {
//! public:
//!   using result_type = R;
//!
//!   // \ref{func.wrap.move.ctor}, constructors, assignments, and destructor
//!   constexpr move_only_function() noexcept;
//!   constexpr move_only_function(nullptr_t) noexcept;
//!   constexpr move_only_function(move_only_function&&) noexcept;
//!   template<class F> constexpr move_only_function(F&&);
//!   template<class T, class... Args>
//!     constexpr explicit move_only_function(in_place_type_t<T>, Args&&...);
//!   template<class T, class U, class... Args>
//!     constexpr explicit move_only_function(in_place_type_t<T>, initializer_list<U>, Args&&...);
//!
//!   constexpr move_only_function& operator=(move_only_function&&);
//!   constexpr move_only_function& operator=(nullptr_t) noexcept;
//!   template<class F> constexpr move_only_function& operator=(F&&);
//!
//!   constexpr ~move_only_function();
//!
//!   // \ref{func.wrap.move.inv}, invocation
//!   constexpr explicit operator bool() const noexcept;
//!   constexpr R operator()(ArgTypes...) @\cv{}@ @\placeholder{ref}@ noexcept(@\placeholder{noex}@);
//!
//!   // \ref{func.wrap.move.util}, utility
//!   constexpr void swap(move_only_function&) noexcept;
//!   friend constexpr void swap(move_only_function&, move_only_function&) noexcept;
//!   friend constexpr bool operator==(const move_only_function&, nullptr_t) noexcept;
//!
//! private:
//!   template<class VT>
//!     static constexpr bool @\exposid{is-callable-from}@ = @\seebelow@;       // \expos
//! };
template <class Sig>
class move_only_function<Sig> : public detail::call_base<move_only_function<Sig>, Sig> {
    //! \omit
    using base = detail::call_base<move_only_function<Sig>, Sig>;
    friend base;

    // \ref{func.wrap.move.ctor}, constructors, assignments, and destructor

    //! If \placeholder{noex} is `true`, `is_callable_from<VT>` is equal to:
    //! \codeblock
    //! is_nothrow_invocable_r_v<R, VT @\cv{}@ @\placeholder{ref}@, ArgTypes...> &&
    //! is_nothrow_invocable_r_v<R, VT @\placeholder{inv-quals}@, ArgTypes...>
    //! \endcodeblock
    //! Otherwise, `is_callable_from<VT>` is equal to:
    //! \codeblock
    //! is_invocable_r_v<R, VT @\cv{}@ @\placeholder{ref}@, ArgTypes...> &&
    //! is_invocable_r_v<R, VT @\placeholder{inv-quals}@, ArgTypes...>
    //! \endcodeblock
    //! \expos
    //! \seebelow
    template <class VT>
    static constexpr bool is_callable_from = base::template is_callable_from<VT>;

  public:
    //! \asdraft
    constexpr move_only_function() noexcept {}
    //! \also
    constexpr move_only_function(std::nullptr_t) noexcept {}

    //! \asdraft
    constexpr move_only_function(move_only_function&& f) noexcept { take(f); }

    //! \asdraft
    //! \covers constraints !detail::is_in_place_type<remove_cvref_t<F>>
    template <class F>
        requires(!std::is_same_v<std::remove_cvref_t<F>, move_only_function>) &&
                (!detail::is_in_place_type<std::remove_cvref_t<F>>) && (is_callable_from<std::decay_t<F>>)
    constexpr move_only_function(F&& f) {
        using VT = std::decay_t<F>;
        static_assert(std::is_constructible_v<VT, F>, "move_only_function target must be constructible from F");
        if constexpr (std::is_pointer_v<std::remove_cvref_t<F>> || std::is_member_pointer_v<VT>) {
            if (f == nullptr)
                return;
        } else if constexpr (detail::is_move_only_function<std::remove_cvref_t<F>> ||
                             detail::is_copyable_function<std::remove_cvref_t<F>>) {
            if (!f)
                return;
        }
        emplace<VT>(std::forward<F>(f));
    }

    //! \asdraft
    template <class T, class... Args>
        requires std::is_constructible_v<std::decay_t<T>, Args...> && (is_callable_from<std::decay_t<T>>)
    constexpr explicit move_only_function(std::in_place_type_t<T>, Args&&... args) {
        static_assert(std::is_same_v<std::decay_t<T>, T>,
                      "move_only_function in_place_type_t<T> requires decay_t<T> == T");
        emplace<T>(std::forward<Args>(args)...);
    }

    //! \asdraft
    template <class T, class U, class... Args>
        requires std::is_constructible_v<std::decay_t<T>, std::initializer_list<U>&, Args...> &&
                 (is_callable_from<std::decay_t<T>>)
    constexpr explicit move_only_function(std::in_place_type_t<T>, std::initializer_list<U> ilist, Args&&... args) {
        static_assert(std::is_same_v<std::decay_t<T>, T>,
                      "move_only_function in_place_type_t<T> requires decay_t<T> == T");
        emplace<T>(ilist, std::forward<Args>(args)...);
    }

    //! \asdraft
    constexpr move_only_function& operator=(move_only_function&& f) {
        move_only_function(std::move(f)).swap(*this);
        return *this;
    }

    //! \asdraft
    constexpr move_only_function& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    //! \asdraft
    template <class F>
    constexpr move_only_function& operator=(F&& f) {
        move_only_function(std::forward<F>(f)).swap(*this);
        return *this;
    }

    //! \asdraft
    constexpr ~move_only_function() { reset(); }

    // \ref{func.wrap.move.inv}, invocation

    //! \asdraft
    constexpr explicit operator bool() const noexcept { return has_target(); }

    // \ref{func.wrap.move.util}, utility

    //! \asdraft
    constexpr void swap(move_only_function& other) noexcept {
        if (this == &other)
            return;
        move_only_function tmp(std::move(other));
        other.take(*this);
        take(tmp);
    }

    //! \asdraft
    friend constexpr void swap(move_only_function& f1, move_only_function& f2) noexcept { f1.swap(f2); }

    //! \asdraft
    friend constexpr bool operator==(const move_only_function& f, std::nullptr_t) noexcept { return !f; }

  private:
    //! \omit
    template <class VT, class... Args>
    constexpr void emplace(Args&&... args) {
        if consteval {
            emplace_as<VT, false>(std::forward<Args>(args)...);
        } else {
            emplace_as<VT, detail::stored_locally<VT>>(std::forward<Args>(args)...);
        }
    }

    //! \omit
    template <class VT, bool Local, class... Args>
    constexpr void emplace_as(Args&&... args) {
        detail::construct_target<VT, Local>(storage_, std::forward<Args>(args)...);
        invoker_ = &base::template invoke_target<VT, Local>;
        manager_ = &detail::manage<VT, Local>;
    }

    // Precondition: *this has no target object.
    //! \omit
    constexpr void take(move_only_function& other) noexcept {
        if (other.has_target())
            other.manager_(detail::manage_op::move, storage_, &other.storage_);
        invoker_ = std::exchange(other.invoker_, nullptr);
        manager_ = std::exchange(other.manager_, nullptr);
    }

    //! \omit
    constexpr void reset() noexcept {
        if (has_target())
            manager_(detail::manage_op::destroy, storage_, nullptr);
        invoker_      = nullptr;
        manager_      = nullptr;
        storage_.heap = nullptr;
    }

    // During constant evaluation every target is on the heap, so the heap
    // pointer tells whether there is one. Testing manager_ instead would
    // compare the address of a function with null, which gcc will not
    // constant-fold under -fno-delete-null-pointer-checks, implied by the
    // null-checking sanitizers in -fsanitize=undefined.

    //! \omit
    constexpr bool has_target() const noexcept {
        if consteval {
            return storage_.heap != nullptr;
        } else {
            return manager_ != nullptr;
        }
    }

    //! \omit
    typename base::invoker_t invoker_ = nullptr;
    //! \omit
    detail::manager_t manager_ = nullptr;
    //! \omit
    detail::callable_storage storage_;
};

// \rSec4[func.wrap.move.ctor]{Constructors, assignments, and destructor}

// \rSec4[func.wrap.move.inv]{Invocation}

// operator() is declared by detail::call_base, with the cv, ref, and noex of
// the signature, so its item is authored here.

//! \asdraft
//! \verbatim-itemdecl
//! constexpr R operator()(ArgTypes... args) @\cv{}@ @\placeholder{ref}@ noexcept(@\placeholder{noex}@);

// \rSec4[func.wrap.move.util]{Utility}

} // namespace beman::constexpr_callables

#endif
