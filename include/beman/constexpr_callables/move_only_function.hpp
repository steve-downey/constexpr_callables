// move_only_function.hpp                                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_CONSTEXPR_CALLABLES_MOVE_ONLY_FUNCTION
#define INCLUDED_BEMAN_CONSTEXPR_CALLABLES_MOVE_ONLY_FUNCTION

#include <beman/constexpr_callables/detail/call_base.hpp>

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace smd {
namespace constexpr_callables {

// [func.wrap.move], move-only wrapper

template <class... S>
class move_only_function; // not defined

namespace detail {
template <class T>
inline constexpr bool is_move_only_function = false;

template <class... S>
inline constexpr bool is_move_only_function<move_only_function<S...>> = true;
} // namespace detail

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

All twelve are provided by this one partial specialization; the parts that
vary with cv, ref, and noex come from detail::call_base<..., Sig>, which is
defined only for those twelve forms of Sig.
*/
template <class Sig>
class move_only_function<Sig> : public detail::call_base<move_only_function<Sig>, Sig> {
    using base = detail::call_base<move_only_function<Sig>, Sig>;
    friend base;

  public:
    // [func.wrap.move.ctor], constructors, assignment, and destructor
    constexpr move_only_function() noexcept = default;
    constexpr move_only_function(std::nullptr_t) noexcept {}
    constexpr move_only_function(move_only_function&& f) noexcept { take(f); }

    template <class F>
        requires(!std::is_same_v<std::remove_cvref_t<F>, move_only_function>) &&
                (!detail::is_in_place_type<std::remove_cvref_t<F>>) &&
                (base::template is_callable_from<std::decay_t<F>>)
    constexpr move_only_function(F&& f) {
        using VT = std::decay_t<F>;
        static_assert(std::is_constructible_v<VT, F>, "move_only_function target must be constructible from F");
        if constexpr (std::is_pointer_v<std::remove_cvref_t<F>> || std::is_member_pointer_v<VT>) {
            if (f == nullptr)
                return;
        } else if constexpr (detail::is_move_only_function<std::remove_cvref_t<F>>) {
            if (!f)
                return;
        }
        emplace<VT>(std::forward<F>(f));
    }

    template <class T, class... Args>
        requires std::is_constructible_v<std::decay_t<T>, Args...> &&
                 (base::template is_callable_from<std::decay_t<T>>)
    constexpr explicit move_only_function(std::in_place_type_t<T>, Args&&... args) {
        static_assert(std::is_same_v<std::decay_t<T>, T>,
                      "move_only_function in_place_type_t<T> requires decay_t<T> == T");
        emplace<T>(std::forward<Args>(args)...);
    }

    template <class T, class U, class... Args>
        requires std::is_constructible_v<std::decay_t<T>, std::initializer_list<U>&, Args...> &&
                 (base::template is_callable_from<std::decay_t<T>>)
    constexpr explicit move_only_function(std::in_place_type_t<T>, std::initializer_list<U> ilist, Args&&... args) {
        static_assert(std::is_same_v<std::decay_t<T>, T>,
                      "move_only_function in_place_type_t<T> requires decay_t<T> == T");
        emplace<T>(ilist, std::forward<Args>(args)...);
    }

    constexpr move_only_function& operator=(move_only_function&& f) {
        move_only_function(std::move(f)).swap(*this);
        return *this;
    }

    constexpr move_only_function& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    template <class F>
    constexpr move_only_function& operator=(F&& f) {
        move_only_function(std::forward<F>(f)).swap(*this);
        return *this;
    }

    constexpr ~move_only_function() { reset(); }

    // [func.wrap.move.inv], invocation
    constexpr explicit operator bool() const noexcept { return has_target(); }

    // [func.wrap.move.util], utility
    constexpr void swap(move_only_function& other) noexcept {
        if (this == &other)
            return;
        move_only_function tmp(std::move(other));
        other.take(*this);
        take(tmp);
    }

    friend constexpr void swap(move_only_function& f1, move_only_function& f2) noexcept { f1.swap(f2); }

    friend constexpr bool operator==(const move_only_function& f, std::nullptr_t) noexcept { return !f; }

  private:
    template <class VT, class... Args>
    constexpr void emplace(Args&&... args) {
        if consteval {
            emplace_as<VT, false>(std::forward<Args>(args)...);
        } else {
            emplace_as<VT, detail::stored_locally<VT>>(std::forward<Args>(args)...);
        }
    }

    template <class VT, bool Local, class... Args>
    constexpr void emplace_as(Args&&... args) {
        detail::construct_target<VT, Local>(storage_, std::forward<Args>(args)...);
        invoker_ = &base::template invoke_target<VT, Local>;
        manager_ = &detail::manage<VT, Local>;
    }

    // Precondition: *this has no target object.
    constexpr void take(move_only_function& other) noexcept {
        if (other.has_target())
            other.manager_(detail::manage_op::move, storage_, &other.storage_);
        invoker_ = std::exchange(other.invoker_, nullptr);
        manager_ = std::exchange(other.manager_, nullptr);
    }

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
    constexpr bool has_target() const noexcept {
        if consteval {
            return storage_.heap != nullptr;
        } else {
            return manager_ != nullptr;
        }
    }

    typename base::invoker_t invoker_ = nullptr;
    detail::manager_t        manager_ = nullptr;
    detail::callable_storage storage_;
};

} // namespace constexpr_callables
} // namespace smd

#endif
