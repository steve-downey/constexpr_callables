// detail/function_ref_impl.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// One partial specialization of function_ref, for the cv placeholder given by
// SMD_CONSTEXPR_CALLABLES_CV; noex is deduced. Included by function_ref.hpp
// once per value of cv, rather than stamped out by a macro so that each line
// keeps its own location for diagnostics and coverage. Without
// SMD_CONSTEXPR_CALLABLES_FUNCTION_REF_IMPL defined it is empty, so it can
// still be compiled on its own.

#ifdef SMD_CONSTEXPR_CALLABLES_FUNCTION_REF_IMPL

template <class R, class... ArgTypes, bool noex>
class function_ref<R(ArgTypes...) SMD_CONSTEXPR_CALLABLES_CV noexcept(noex)> {
    template <class... T>
    static constexpr bool is_invocable_using = detail::invocable_using<noex, R, T..., ArgTypes...>;

    template <class F>
    static constexpr bool is_convertible_from_specialization = detail::is_convertible_function_ref<F, function_ref>;

    // A specialization converted from another copies its bound entity and
    // thunk, so each must be able to read the other's.
    template <class... S>
    friend class function_ref;

  public:
    // [func.wrap.ref.ctor], constructors and assignment operators
    template <class F>
        requires std::is_function_v<F> && (is_invocable_using<F>)
    function_ref(F* f) noexcept : thunk_ptr_(&function_thunk<F>) {
        bound_entity_.function = reinterpret_cast<void (*)()>(f);
    }

    template <class F, class T = std::remove_reference_t<F>>
        requires(!std::is_same_v<std::remove_cvref_t<F>, function_ref>) && (!std::is_member_pointer_v<T>) &&
                (is_invocable_using<SMD_CONSTEXPR_CALLABLES_CV T&>)
    constexpr function_ref(F&& f) noexcept {
        if constexpr (is_convertible_from_specialization<std::remove_cv_t<T>>) {
            bound_entity_ = f.bound_entity_;
            thunk_ptr_    = f.thunk_ptr_;
        } else if constexpr (std::is_function_v<T>) {
            bound_entity_.function = reinterpret_cast<void (*)()>(std::addressof(f));
            thunk_ptr_             = &function_thunk<T>;
        } else {
            bound_entity_.object = std::addressof(f);
            thunk_ptr_           = &object_thunk<T>;
        }
    }

    template <auto c, class F>
        requires(is_invocable_using<const F&>)
    constexpr function_ref(std::constant_wrapper<c, F>) noexcept
        : thunk_ptr_(&constant_thunk<std::constant_wrapper<c, F>>) {
        static_assert(!detail::constant_wrapper_is_null<std::constant_wrapper<c, F>, F>,
                      "function_ref constant_wrapper<c, F> requires f.value != nullptr");
        static_assert(!detail::constant_wrapper_call_is_constant<std::constant_wrapper<c, F>, ArgTypes...>,
                      "function_ref constant_wrapper<c, F> requires that invoking f.value with "
                      "constexpr-param arguments not yield a constant_wrapper");
    }

    template <auto c, class F, class U, class T = std::remove_reference_t<U>>
        requires(!std::is_rvalue_reference_v<U &&>) && (is_invocable_using<const F&, SMD_CONSTEXPR_CALLABLES_CV T&>)
    constexpr function_ref(std::constant_wrapper<c, F>, U&& obj) noexcept
        : thunk_ptr_(&bound_object_thunk<std::constant_wrapper<c, F>, T>) {
        static_assert(!detail::constant_wrapper_is_null<std::constant_wrapper<c, F>, F>,
                      "function_ref constant_wrapper<c, F> requires f.value != nullptr");
        bound_entity_.object = std::addressof(obj);
    }

    template <auto c, class F, class T>
        requires(is_invocable_using<const F&, SMD_CONSTEXPR_CALLABLES_CV T*>)
    constexpr function_ref(std::constant_wrapper<c, F>, SMD_CONSTEXPR_CALLABLES_CV T* obj) noexcept
        : thunk_ptr_(&bound_pointer_thunk<std::constant_wrapper<c, F>, T>) {
        static_assert(!detail::constant_wrapper_is_null<std::constant_wrapper<c, F>, F>,
                      "function_ref constant_wrapper<c, F> requires f.value != nullptr");
        bound_entity_.object = obj;
    }

    constexpr function_ref(const function_ref&) noexcept            = default;
    constexpr function_ref& operator=(const function_ref&) noexcept = default;

    template <class T>
        requires(!is_convertible_from_specialization<T>) && (!std::is_pointer_v<T>) &&
                    (!detail::is_constant_wrapper<T>)
    function_ref& operator=(T) = delete;

    // [func.wrap.ref.inv], invocation
    constexpr R operator()(ArgTypes... args) const noexcept(noex) {
        return thunk_ptr_(bound_entity_, std::forward<ArgTypes>(args)...);
    }

  private:
    // The bound object pointer is stored as const void*; its constness is
    // restored from T, which carries the cv-qualification of the original.
    template <class T>
    static constexpr T* object_pointer(detail::bound_entity entity) noexcept {
        return const_cast<T*>(static_cast<const T*>(entity.object));
    }

    template <class F>
    static R function_thunk(detail::bound_entity entity, ArgTypes&&... args) noexcept(noex) {
        return std::invoke_r<R>(reinterpret_cast<F*>(entity.function), std::forward<ArgTypes>(args)...);
    }

    template <class T>
    static constexpr R object_thunk(detail::bound_entity entity, ArgTypes&&... args) noexcept(noex) {
        return std::invoke_r<R>(static_cast<SMD_CONSTEXPR_CALLABLES_CV T&>(*object_pointer<T>(entity)),
                                std::forward<ArgTypes>(args)...);
    }

    template <class CW>
    static constexpr R constant_thunk(detail::bound_entity, ArgTypes&&... args) noexcept(noex) {
        return std::invoke_r<R>(CW::value, std::forward<ArgTypes>(args)...);
    }

    template <class CW, class T>
    static constexpr R bound_object_thunk(detail::bound_entity entity, ArgTypes&&... args) noexcept(noex) {
        return std::invoke_r<R>(CW::value,
                                static_cast<SMD_CONSTEXPR_CALLABLES_CV T&>(*object_pointer<T>(entity)),
                                std::forward<ArgTypes>(args)...);
    }

    template <class CW, class T>
    static constexpr R bound_pointer_thunk(detail::bound_entity entity, ArgTypes&&... args) noexcept(noex) {
        return std::invoke_r<R>(CW::value,
                                static_cast<SMD_CONSTEXPR_CALLABLES_CV T*>(object_pointer<T>(entity)),
                                std::forward<ArgTypes>(args)...);
    }

    R (*thunk_ptr_)(detail::bound_entity, ArgTypes&&...) noexcept(noex);
    detail::bound_entity bound_entity_;
};

#endif
