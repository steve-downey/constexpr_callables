// detail/call_base.hpp                                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_CONSTEXPR_CALLABLES_DETAIL_CALL_BASE
#define INCLUDED_BEMAN_CONSTEXPR_CALLABLES_DETAIL_CALL_BASE

// Type-erasure machinery shared by the owning wrappers, move_only_function
// and copyable_function.
//
// A target object lives either in a small local buffer or on the heap. During
// constant evaluation it is always on the heap: a constant expression may
// allocate and may cast a void* back to the type it came from, but it may not
// create an object of arbitrary type inside a byte buffer. At runtime the
// local buffer is used for small, nothrow-movable targets, per the
// recommended practice in [func.wrap.move.class] and [func.wrap.copy.class].
// The two evaluation modes therefore take different paths, and each is
// selected by `if consteval` in the owning wrapper.

#include <cstddef>
#include <functional>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace smd::constexpr_callables::detail {

union callable_storage {
    void* heap;
    alignas(std::max_align_t) unsigned char local[3 * sizeof(void*)];

    constexpr callable_storage() noexcept : heap(nullptr) {}
};

template <class VT>
inline constexpr bool stored_locally =
    sizeof(VT) <= sizeof(callable_storage::local) && alignof(VT) <= alignof(callable_storage) &&
    std::is_nothrow_move_constructible_v<VT>;

template <class VT, bool Local>
constexpr VT* target(const callable_storage& storage) noexcept {
    if constexpr (Local) {
        return std::launder(reinterpret_cast<VT*>(const_cast<unsigned char*>(storage.local)));
    } else {
        return static_cast<VT*>(storage.heap);
    }
}

template <class VT, bool Local, class... Args>
constexpr void construct_target(callable_storage& storage, Args&&... args) {
    if constexpr (Local) {
        ::new (static_cast<void*>(storage.local)) VT(std::forward<Args>(args)...);
    } else {
        storage.heap = new VT(std::forward<Args>(args)...);
    }
}

enum class manage_op { destroy, move, copy };

// destroy: end the target in `self`.
// move:    move the target of `*other` into `self`; `*other` no longer has one.
// copy:    copy the target of `*other` into `self`.
using manager_t = void (*)(manage_op, callable_storage& self, callable_storage* other);

template <class VT, bool Local>
constexpr void manage(manage_op op, callable_storage& self, callable_storage* other) {
    switch (op) {
    case manage_op::destroy:
        if constexpr (Local) {
            std::destroy_at(target<VT, true>(self));
        } else {
            delete target<VT, false>(self);
        }
        return;
    case manage_op::move:
        if constexpr (Local) {
            VT* source = target<VT, true>(*other);
            construct_target<VT, true>(self, std::move(*source));
            std::destroy_at(source);
        } else {
            self.heap   = other->heap;
            other->heap = nullptr;
        }
        return;
    case manage_op::copy:
        if constexpr (std::is_copy_constructible_v<VT>) {
            construct_target<VT, Local>(self, std::as_const(*target<VT, Local>(*other)));
        }
        return;
    }
}

template <bool Noex, class R, class F, class... Args>
inline constexpr bool invocable_r = std::is_invocable_r_v<R, F, Args...>;

template <class R, class F, class... Args>
inline constexpr bool invocable_r<true, R, F, Args...> = std::is_nothrow_invocable_r_v<R, F, Args...>;

// call_base<Derived, Sig> provides the parts of an owning wrapper that depend
// on the cv, ref, and noex placeholders of Sig: the call operator with exactly
// those qualifiers, is-callable-from, and the invoker that calls a target of
// type VT as `VT inv-quals`. Derived supplies `invoker_` and `storage_`.
template <class Derived, class Sig>
class call_base; // not defined

#define SMD_CONSTEXPR_CALLABLES_CALL_BASE_IMPL

#define SMD_CONSTEXPR_CALLABLES_CV
#define SMD_CONSTEXPR_CALLABLES_REF
#define SMD_CONSTEXPR_CALLABLES_INV_QUALS &
#include <beman/constexpr_callables/detail/call_base_impl.hpp>
#undef SMD_CONSTEXPR_CALLABLES_CV
#undef SMD_CONSTEXPR_CALLABLES_REF
#undef SMD_CONSTEXPR_CALLABLES_INV_QUALS

#define SMD_CONSTEXPR_CALLABLES_CV
#define SMD_CONSTEXPR_CALLABLES_REF &
#define SMD_CONSTEXPR_CALLABLES_INV_QUALS &
#include <beman/constexpr_callables/detail/call_base_impl.hpp>
#undef SMD_CONSTEXPR_CALLABLES_CV
#undef SMD_CONSTEXPR_CALLABLES_REF
#undef SMD_CONSTEXPR_CALLABLES_INV_QUALS

#define SMD_CONSTEXPR_CALLABLES_CV
#define SMD_CONSTEXPR_CALLABLES_REF &&
#define SMD_CONSTEXPR_CALLABLES_INV_QUALS &&
#include <beman/constexpr_callables/detail/call_base_impl.hpp>
#undef SMD_CONSTEXPR_CALLABLES_CV
#undef SMD_CONSTEXPR_CALLABLES_REF
#undef SMD_CONSTEXPR_CALLABLES_INV_QUALS

#define SMD_CONSTEXPR_CALLABLES_CV const
#define SMD_CONSTEXPR_CALLABLES_REF
#define SMD_CONSTEXPR_CALLABLES_INV_QUALS const&
#include <beman/constexpr_callables/detail/call_base_impl.hpp>
#undef SMD_CONSTEXPR_CALLABLES_CV
#undef SMD_CONSTEXPR_CALLABLES_REF
#undef SMD_CONSTEXPR_CALLABLES_INV_QUALS

#define SMD_CONSTEXPR_CALLABLES_CV const
#define SMD_CONSTEXPR_CALLABLES_REF &
#define SMD_CONSTEXPR_CALLABLES_INV_QUALS const&
#include <beman/constexpr_callables/detail/call_base_impl.hpp>
#undef SMD_CONSTEXPR_CALLABLES_CV
#undef SMD_CONSTEXPR_CALLABLES_REF
#undef SMD_CONSTEXPR_CALLABLES_INV_QUALS

#define SMD_CONSTEXPR_CALLABLES_CV const
#define SMD_CONSTEXPR_CALLABLES_REF &&
#define SMD_CONSTEXPR_CALLABLES_INV_QUALS const&&
#include <beman/constexpr_callables/detail/call_base_impl.hpp>
#undef SMD_CONSTEXPR_CALLABLES_CV
#undef SMD_CONSTEXPR_CALLABLES_REF
#undef SMD_CONSTEXPR_CALLABLES_INV_QUALS

#undef SMD_CONSTEXPR_CALLABLES_CALL_BASE_IMPL

template <class T>
inline constexpr bool is_in_place_type = false;

template <class T>
inline constexpr bool is_in_place_type<std::in_place_type_t<T>> = true;

} // namespace smd::constexpr_callables::detail

#endif
