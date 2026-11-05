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
        if constexpr (std::is_function_v<T>) {
            bound_entity_.function = reinterpret_cast<void (*)()>(std::addressof(f));
            thunk_ptr_             = &function_thunk<T>;
        } else {
            bound_entity_.object = std::addressof(f);
            thunk_ptr_           = &object_thunk<T>;
        }
    }

    template <auto f>
        requires(is_invocable_using<decltype(f)>)
    constexpr function_ref(nontype_t<f>) noexcept : thunk_ptr_(&nontype_thunk<f>) {
        static_assert(!detail::nontype_is_null<f>, "function_ref nontype_t<f> requires f != nullptr");
    }

    template <auto f, class U, class T = std::remove_reference_t<U>>
        requires(!std::is_rvalue_reference_v<U &&>) && (is_invocable_using<decltype(f), SMD_CONSTEXPR_CALLABLES_CV T&>)
    constexpr function_ref(nontype_t<f>, U&& obj) noexcept : thunk_ptr_(&bound_object_thunk<f, T>) {
        static_assert(!detail::nontype_is_null<f>, "function_ref nontype_t<f> requires f != nullptr");
        bound_entity_.object = std::addressof(obj);
    }

    template <auto f, class T>
        requires(is_invocable_using<decltype(f), SMD_CONSTEXPR_CALLABLES_CV T*>)
    constexpr function_ref(nontype_t<f>, SMD_CONSTEXPR_CALLABLES_CV T* obj) noexcept : thunk_ptr_(&bound_pointer_thunk<f, T>) {
        static_assert(!detail::nontype_is_null<f>, "function_ref nontype_t<f> requires f != nullptr");
        bound_entity_.object = obj;
    }

    constexpr function_ref(const function_ref&) noexcept            = default;
    constexpr function_ref& operator=(const function_ref&) noexcept = default;

    template <class T>
        requires(!std::is_same_v<T, function_ref>) && (!std::is_pointer_v<T>) && (!detail::is_nontype<T>)
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

    template <auto f>
    static constexpr R nontype_thunk(detail::bound_entity, ArgTypes&&... args) noexcept(noex) {
        return std::invoke_r<R>(f, std::forward<ArgTypes>(args)...);
    }

    template <auto f, class T>
    static constexpr R bound_object_thunk(detail::bound_entity entity, ArgTypes&&... args) noexcept(noex) {
        return std::invoke_r<R>(
            f, static_cast<SMD_CONSTEXPR_CALLABLES_CV T&>(*object_pointer<T>(entity)), std::forward<ArgTypes>(args)...);
    }

    template <auto f, class T>
    static constexpr R bound_pointer_thunk(detail::bound_entity entity, ArgTypes&&... args) noexcept(noex) {
        return std::invoke_r<R>(
            f, static_cast<SMD_CONSTEXPR_CALLABLES_CV T*>(object_pointer<T>(entity)), std::forward<ArgTypes>(args)...);
    }

    R (*thunk_ptr_)(detail::bound_entity, ArgTypes&&...) noexcept(noex);
    detail::bound_entity bound_entity_;
};

#endif
