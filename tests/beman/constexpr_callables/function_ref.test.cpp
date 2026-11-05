// tests/beman/constexpr_callables/function_ref.test.cpp             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/constexpr_callables/function_ref.hpp>

#include <beman/constexpr_callables/function_ref.hpp> // test 2nd include OK

#include <beman/constexpr_callables/testing/constant_eval.hpp>

#include <gtest/gtest.h>

#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>

namespace {

using smd::constexpr_callables::function_ref;
using smd::constexpr_callables::nontype;
using smd::constexpr_callables::nontype_t;
using smd::constexpr_callables::testing::constant_eval;

// ---------------------------------------------------------------------------
// Test callables
// ---------------------------------------------------------------------------

enum class called_as { lvalue = 1, const_lvalue };

struct qualified {
    constexpr called_as operator()() & { return called_as::lvalue; }
    constexpr called_as operator()() const& { return called_as::const_lvalue; }
};

struct accumulator {
    int           total = 0;
    constexpr int add(int x) { return total += x; }
    constexpr int get() const { return total; }
    constexpr int get_noexcept() const noexcept { return total; }
    constexpr int add_lref(int x) & { return total += x; }
};

constexpr int twice(int x) { return 2 * x; }
constexpr int add_to(accumulator* acc, int x) { return acc->add(x); }
constexpr int add_to_ref(accumulator& acc, int x) { return acc.add(x); }
constexpr int increment(int x) noexcept { return x + 1; }

int runtime_triple(int x) { return 3 * x; }
int runtime_noexcept(int x) noexcept { return x - 1; }

} // namespace

// ---------------------------------------------------------------------------
// [func.wrap.ref.class]
// ---------------------------------------------------------------------------

static_assert(std::is_trivially_copyable_v<function_ref<int(int)>>);
static_assert(std::is_trivially_copyable_v<function_ref<int(int) const noexcept>>);
static_assert(std::copyable<function_ref<int(int)>>);
static_assert(std::copyable<function_ref<void() const>>);
static_assert(!std::is_default_constructible_v<function_ref<int()>>);

// The call operator is const, and carries the noexcept of the signature.
static_assert(!noexcept(std::declval<const function_ref<int()>&>()()));
static_assert(noexcept(std::declval<const function_ref<int() noexcept>&>()()));
static_assert(noexcept(std::declval<const function_ref<int() const noexcept>&>()()));

// ---------------------------------------------------------------------------
// [func.wrap.ref.ctor] is-invocable-using
// ---------------------------------------------------------------------------

namespace {
struct non_const_only {
    int operator()() { return 0; }
};
struct may_throw {
    int operator()() const { return 0; }
};
} // namespace

static_assert(std::is_constructible_v<function_ref<int()>, non_const_only&>);
static_assert(!std::is_constructible_v<function_ref<int() const>, non_const_only&>);
static_assert(!std::is_constructible_v<function_ref<int() noexcept>, may_throw&>);
static_assert(std::is_constructible_v<function_ref<void()>, may_throw&>);
static_assert(std::is_constructible_v<function_ref<long()>, may_throw&>);
static_assert(!std::is_constructible_v<function_ref<int*()>, may_throw&>);

// ---------------------------------------------------------------------------
// [func.wrap.ref.ctor] function_ref(F*): runtime only
// ---------------------------------------------------------------------------
// Binding a function pointer needs a reinterpret_cast, so it is not available
// in constant expressions; a negative test pins that down.

TEST(FunctionRef, RuntimeFunctionPointer) {
    function_ref<int(int)> f = &runtime_triple;
    EXPECT_EQ(f(2), 6);
}

TEST(FunctionRef, RuntimeFunctionPointerNoexcept) {
    function_ref<int(int) noexcept> f = &runtime_noexcept;
    EXPECT_EQ(f(2), 1);
}

TEST(FunctionRef, RuntimeFunctionPointerConstSignature) {
    function_ref<int(int) const> f = &runtime_triple;
    EXPECT_EQ(f(3), 9);
}

TEST(FunctionRef, RuntimeFunctionPointerConvertsResult) {
    function_ref<long(int)> f = &runtime_triple;
    EXPECT_EQ(f(4), 12L);
}

TEST(FunctionRef, RuntimeFunctionPointerDiscardsResultForVoid) {
    function_ref<void(int)> f = &runtime_triple;
    f(4);
    SUCCEED();
}

// ---------------------------------------------------------------------------
// [func.wrap.ref.ctor] function_ref(F&&)
// ---------------------------------------------------------------------------

TEST(FunctionRef, BindsLambdaLvalue) {
    constexpr auto probe = [] {
        int                    k = 3;
        auto                   l = [k](int x) { return x * k; };
        function_ref<int(int)> f = l;
        return f(5);
    };
    EXPECT_EQ(constant_eval(probe), 15);
    EXPECT_EQ(probe(), 15);
}

TEST(FunctionRef, BindsTemporaryForTheFullExpression) {
    constexpr auto probe = [] {
        auto call = [](function_ref<int(int)> f) { return f(2); };
        return call([](int x) { return x + 40; });
    };
    EXPECT_EQ(constant_eval(probe), 42);
    EXPECT_EQ(probe(), 42);
}

TEST(FunctionRef, RefersToTheObjectNotACopy) {
    constexpr auto probe = [] {
        int                    total = 0;
        auto                   add   = [&total](int x) { return total += x; };
        function_ref<int(int)> f     = add;
        f(2);
        f(3);
        return total;
    };
    EXPECT_EQ(constant_eval(probe), 5);
    EXPECT_EQ(probe(), 5);
}

// The object is called as `cv T&`.
TEST(FunctionRef, CallsObjectAsCvLvalue) {
    constexpr auto probe = [] {
        qualified                       q;
        const qualified                 cq;
        function_ref<called_as()>       plain    = q;
        function_ref<called_as() const> c        = q;
        function_ref<called_as()>       of_const = cq;
        return plain() == called_as::lvalue && c() == called_as::const_lvalue && of_const() == called_as::const_lvalue;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(FunctionRef, BindsObjectNoexcept) {
    constexpr auto probe = [] {
        auto                                  l = [](int x) noexcept { return x + 1; };
        function_ref<int(int) const noexcept> f = l;
        return f(1);
    };
    EXPECT_EQ(constant_eval(probe), 2);
    EXPECT_EQ(probe(), 2);
}

TEST(FunctionRef, ObjectConvertsResultAndDiscardsForVoid) {
    constexpr auto probe = [] {
        int  calls = 0;
        auto l     = [&calls](int x) {
            ++calls;
            return x;
        };
        function_ref<long(int)> as_long = l;
        function_ref<void(int)> as_void = l;
        long                    r       = as_long(7);
        as_void(8);
        return r * 10 + calls;
    };
    EXPECT_EQ(constant_eval(probe), 72L);
    EXPECT_EQ(probe(), 72L);
}

TEST(FunctionRef, ForwardsArguments) {
    constexpr auto probe = [] {
        auto l = [](std::unique_ptr<int> p, int& out) {
            out = *p;
            return *p + 1;
        };
        function_ref<int(std::unique_ptr<int>, int&)> f      = l;
        int                                           out    = 0;
        int                                           result = f(std::make_unique<int>(4), out);
        return result * 10 + out;
    };
    EXPECT_EQ(constant_eval(probe), 54);
    EXPECT_EQ(probe(), 54);
}

TEST(FunctionRef, RuntimeBindsFunctionLvalue) {
    function_ref<int(int)> f = runtime_triple;
    EXPECT_EQ(f(5), 15);
}

// ---------------------------------------------------------------------------
// [func.wrap.ref.ctor] function_ref(nontype_t<f>)
// ---------------------------------------------------------------------------

TEST(FunctionRef, NontypeFunctionPointer) {
    constexpr auto probe = [] {
        function_ref<int(int)> f(nontype<&twice>);
        return f(21);
    };
    EXPECT_EQ(constant_eval(probe), 42);
    EXPECT_EQ(probe(), 42);
}

TEST(FunctionRef, NontypeLambda) {
    constexpr auto probe = [] {
        function_ref<int(int) const noexcept> f(nontype<[](int x) noexcept { return x * x; }>);
        return f(6);
    };
    EXPECT_EQ(constant_eval(probe), 36);
    EXPECT_EQ(probe(), 36);
}

TEST(FunctionRef, NontypeMemberFunctionTakesObjectAsArgument) {
    constexpr auto probe = [] {
        accumulator                          acc;
        function_ref<int(accumulator&, int)> f(nontype<&accumulator::add>);
        f(acc, 2);
        return f(acc, 3);
    };
    EXPECT_EQ(constant_eval(probe), 5);
    EXPECT_EQ(probe(), 5);
}

// ---------------------------------------------------------------------------
// [func.wrap.ref.ctor] function_ref(nontype_t<f>, U&&)
// ---------------------------------------------------------------------------

TEST(FunctionRef, NontypeBoundToObjectReference) {
    constexpr auto probe = [] {
        accumulator            acc;
        function_ref<int(int)> f(nontype<&accumulator::add>, acc);
        f(2);
        f(3);
        return acc.total;
    };
    EXPECT_EQ(constant_eval(probe), 5);
    EXPECT_EQ(probe(), 5);
}

TEST(FunctionRef, NontypeBoundToConstObjectReference) {
    constexpr auto probe = [] {
        const accumulator         acc{.total = 9};
        function_ref<int() const> f(nontype<&accumulator::get>, acc);
        return f();
    };
    EXPECT_EQ(constant_eval(probe), 9);
    EXPECT_EQ(probe(), 9);
}

TEST(FunctionRef, NontypeBoundToObjectForFreeFunction) {
    constexpr auto probe = [] {
        accumulator            acc;
        function_ref<int(int)> f(nontype<&add_to_ref>, acc);
        f(4);
        return acc.total;
    };
    EXPECT_EQ(constant_eval(probe), 4);
    EXPECT_EQ(probe(), 4);
}

TEST(FunctionRef, NontypeBoundToObjectMemberData) {
    constexpr auto probe = [] {
        accumulator         acc{.total = 11};
        function_ref<int()> f(nontype<&accumulator::total>, acc);
        return f();
    };
    EXPECT_EQ(constant_eval(probe), 11);
    EXPECT_EQ(probe(), 11);
}

// ---------------------------------------------------------------------------
// [func.wrap.ref.ctor] function_ref(nontype_t<f>, cv T*)
// ---------------------------------------------------------------------------

TEST(FunctionRef, NontypeBoundToPointerForMemberFunction) {
    constexpr auto probe = [] {
        accumulator            acc;
        function_ref<int(int)> f(nontype<&accumulator::add>, &acc);
        f(6);
        return acc.total;
    };
    EXPECT_EQ(constant_eval(probe), 6);
    EXPECT_EQ(probe(), 6);
}

TEST(FunctionRef, NontypeBoundToPointerForFreeFunction) {
    constexpr auto probe = [] {
        accumulator            acc;
        function_ref<int(int)> f(nontype<&add_to>, &acc);
        f(7);
        return acc.total;
    };
    EXPECT_EQ(constant_eval(probe), 7);
    EXPECT_EQ(probe(), 7);
}

TEST(FunctionRef, NontypeBoundToConstPointer) {
    constexpr auto probe = [] {
        const accumulator         acc{.total = 8};
        function_ref<int() const> f(nontype<&accumulator::get>, &acc);
        return f();
    };
    EXPECT_EQ(constant_eval(probe), 8);
    EXPECT_EQ(probe(), 8);
}

// ---------------------------------------------------------------------------
// [func.wrap.ref.ctor] copy and assignment
// ---------------------------------------------------------------------------

TEST(FunctionRef, CopyRefersToTheSameEntity) {
    constexpr auto probe = [] {
        int                    total = 0;
        auto                   add   = [&total](int x) { return total += x; };
        function_ref<int(int)> f     = add;
        function_ref<int(int)> g     = f;
        f(1);
        g(2);
        return total;
    };
    EXPECT_EQ(constant_eval(probe), 3);
    EXPECT_EQ(probe(), 3);
}

TEST(FunctionRef, CopyAssignmentRebinds) {
    constexpr auto probe = [] {
        auto                   one          = [](int) { return 1; };
        auto                   two          = [](int) { return 2; };
        function_ref<int(int)> f            = one;
        function_ref<int(int)> g            = two;
        bool                   returns_this = &(f = g) == &f;
        return returns_this ? f(0) : -1;
    };
    EXPECT_EQ(constant_eval(probe), 2);
    EXPECT_EQ(probe(), 2);
}

// operator=(T) is deleted only for T that is not function_ref, a pointer, or
// a nontype_t; those assign through a converting constructor.
TEST(FunctionRef, AssignNontypeRebinds) {
    constexpr auto probe = [] {
        auto                   one = [](int) { return 1; };
        function_ref<int(int)> f   = one;
        f                          = nontype<&twice>;
        return f(4);
    };
    EXPECT_EQ(constant_eval(probe), 8);
    EXPECT_EQ(probe(), 8);
}

TEST(FunctionRef, RuntimeAssignFunctionPointerRebinds) {
    auto                   one = [](int) { return 1; };
    function_ref<int(int)> f   = one;
    f                          = &runtime_triple;
    EXPECT_EQ(f(2), 6);
}

static_assert(!std::is_assignable_v<function_ref<int(int)>&, decltype([](int x) { return x; })>);
static_assert(std::is_assignable_v<function_ref<int(int)>&, int (*)(int)>);
static_assert(std::is_assignable_v<function_ref<int(int)>&, nontype_t<&twice>>);
static_assert(std::is_assignable_v<function_ref<int(int)>&, const function_ref<int(int)>&>);

// ---------------------------------------------------------------------------
// [func.wrap.ref.deduct]
// ---------------------------------------------------------------------------

TEST(FunctionRef, RuntimeDeduceFromFunctionPointer) {
    function_ref f(&runtime_triple);
    static_assert(std::is_same_v<decltype(f), function_ref<int(int)>>);
    function_ref n(&runtime_noexcept);
    static_assert(std::is_same_v<decltype(n), function_ref<int(int) noexcept>>);
    EXPECT_EQ(f(1) + n(1), 3);
}

TEST(FunctionRef, DeduceFromNontypeFunction) {
    constexpr auto probe = [] {
        function_ref f(nontype<&twice>);
        static_assert(std::is_same_v<decltype(f), function_ref<int(int)>>);
        function_ref n(nontype<&increment>);
        static_assert(std::is_same_v<decltype(n), function_ref<int(int) noexcept>>);
        return f(2) + n(2);
    };
    EXPECT_EQ(constant_eval(probe), 7);
    EXPECT_EQ(probe(), 7);
}

TEST(FunctionRef, DeduceFromNontypeMemberFunctionAndObject) {
    constexpr auto probe = [] {
        accumulator  acc;
        function_ref add(nontype<&accumulator::add>, acc);
        static_assert(std::is_same_v<decltype(add), function_ref<int(int)>>);
        function_ref get(nontype<&accumulator::get>, acc);
        static_assert(std::is_same_v<decltype(get), function_ref<int()>>);
        function_ref get_noexcept(nontype<&accumulator::get_noexcept>, acc);
        static_assert(std::is_same_v<decltype(get_noexcept), function_ref<int() noexcept>>);
        function_ref add_lref(nontype<&accumulator::add_lref>, acc);
        static_assert(std::is_same_v<decltype(add_lref), function_ref<int(int)>>);
        add(1);
        add_lref(2);
        return get() + get_noexcept();
    };
    EXPECT_EQ(constant_eval(probe), 6);
    EXPECT_EQ(probe(), 6);
}

TEST(FunctionRef, DeduceFromNontypeMemberDataAndObject) {
    constexpr auto probe = [] {
        accumulator  acc{.total = 5};
        function_ref f(nontype<&accumulator::total>, acc);
        static_assert(std::is_same_v<decltype(f), function_ref<int&()>>);
        f() += 1;
        return acc.total;
    };
    EXPECT_EQ(constant_eval(probe), 6);
    EXPECT_EQ(probe(), 6);
}

TEST(FunctionRef, DeduceFromNontypeFunctionAndFirstArgument) {
    constexpr auto probe = [] {
        accumulator  acc;
        function_ref f(nontype<&add_to_ref>, acc);
        static_assert(std::is_same_v<decltype(f), function_ref<int(int)>>);
        f(9);
        return acc.total;
    };
    EXPECT_EQ(constant_eval(probe), 9);
    EXPECT_EQ(probe(), 9);
}
