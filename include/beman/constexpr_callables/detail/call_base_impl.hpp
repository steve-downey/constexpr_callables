// detail/call_base_impl.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// One specialization of call_base, for the cv and ref placeholders given by
// SMD_CONSTEXPR_CALLABLES_CV, SMD_CONSTEXPR_CALLABLES_REF, and
// SMD_CONSTEXPR_CALLABLES_INV_QUALS; noex is deduced. Included by
// call_base.hpp once per combination, inside namespace detail, rather than
// stamped out by a macro so that each line keeps its own location for
// diagnostics and coverage. Without SMD_CONSTEXPR_CALLABLES_CALL_BASE_IMPL
// defined it is empty, so it can still be compiled on its own.

#ifdef SMD_CONSTEXPR_CALLABLES_CALL_BASE_IMPL

template <class Derived, class R, class... ArgTypes, bool Noex>
class call_base<Derived, R(ArgTypes...) SMD_CONSTEXPR_CALLABLES_CV SMD_CONSTEXPR_CALLABLES_REF noexcept(Noex)> {
  public:
    using result_type = R;

    constexpr R operator()(ArgTypes... args) SMD_CONSTEXPR_CALLABLES_CV SMD_CONSTEXPR_CALLABLES_REF noexcept(Noex) {
        const Derived& self = static_cast<const Derived&>(*this);
        return self.invoker_(self.storage_, std::forward<ArgTypes>(args)...);
    }

  protected:
    using invoker_t = R (*)(const callable_storage&, ArgTypes&&...) noexcept(Noex);

    template <class VT>
    static constexpr bool is_callable_from =
        invocable_r<Noex, R, VT SMD_CONSTEXPR_CALLABLES_CV SMD_CONSTEXPR_CALLABLES_REF, ArgTypes...> &&
        invocable_r<Noex, R, VT SMD_CONSTEXPR_CALLABLES_INV_QUALS, ArgTypes...>;

    template <class VT, bool Local>
    static constexpr R invoke_target(const callable_storage& storage, ArgTypes&&... args) noexcept(Noex) {
        return std::invoke_r<R>(static_cast<VT SMD_CONSTEXPR_CALLABLES_INV_QUALS>(*target<VT, Local>(storage)),
                                std::forward<ArgTypes>(args)...);
    }
};

#endif
