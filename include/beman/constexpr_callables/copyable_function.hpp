// copyable_function.hpp                                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_CONSTEXPR_CALLABLES_COPYABLE_FUNCTION
#define INCLUDED_BEMAN_CONSTEXPR_CALLABLES_COPYABLE_FUNCTION

#include <cstddef>
#include <utility>
#include <initializer_list>

namespace smd {
namespace constexpr_callables {
// [func.wrap.copy], copyable wrapper

template <class... S>
class copyable_function; // not defined
// template <class R, class... ArgTypes>
// class copyable_function<R(ArgTypes...) cv ref noexcept(noex)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) noexcept(false)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) noexcept(true)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) & noexcept(false)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) & noexcept(true)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) && noexcept(false)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) && noexcept(true)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) const noexcept(false)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) const noexcept(true)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) const & noexcept(false)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) const & noexcept(true)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) const && noexcept(false)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) const && noexcept(true)>; // see below

template <class R, class... ArgTypes>
class copyable_function<R(ArgTypes...) noexcept(false)> {
  public:
    using result_type = R;

    // [func.wrap.copy.ctor], constructors, assignments, and destructors
    copyable_function() noexcept;
    copyable_function(std::nullptr_t) noexcept;
    copyable_function(const copyable_function&);
    copyable_function(copyable_function&&) noexcept;
    template <class F>
    copyable_function(F&&);
    template <class T, class... Args>
    explicit copyable_function(std::in_place_type_t<T>, Args&&...);
    template <class T, class U, class... Args>
    explicit copyable_function(std::in_place_type_t<T>, std::initializer_list<U>, Args&&...);

    copyable_function& operator=(const copyable_function&);
    copyable_function& operator=(copyable_function&&);
    copyable_function& operator=(std::nullptr_t) noexcept;
    template <class F>
    copyable_function& operator=(F&&);

    ~copyable_function();

    // [func.wrap.copy.inv], invocation
    explicit operator bool() const noexcept;
    R        operator()(ArgTypes...) noexcept(false);

    // [func.wrap.copy.util], utility
    void        swap(copyable_function&) noexcept;
    friend void swap(copyable_function&, copyable_function&) noexcept;
    friend bool operator==(const copyable_function&, std::nullptr_t) noexcept;

  private:
    template <class VT>
    static constexpr bool is_callable_from =
        std::is_invocable_r_v<R, VT, ArgTypes...> && std::is_invocable_r_v<R, VT&, ArgTypes...>;
    // exposition only
};
} // namespace constexpr_callables
} // namespace smd

#endif
