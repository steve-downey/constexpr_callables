// tests/beman/constexpr_callables/copyable_function.test.cpp       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/constexpr_callables/copyable_function.hpp>

#include <beman/constexpr_callables/copyable_function.hpp> // test 2nd include OK

#include <beman/constexpr_callables/testing/constant_eval.hpp>

#include <gtest/gtest.h>

#include <functional>
#include <initializer_list>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace {

using beman::constexpr_callables::copyable_function;
using beman::constexpr_callables::testing::constant_eval;

// ---------------------------------------------------------------------------
// Test callables
// ---------------------------------------------------------------------------

// Records the special member calls made on it, through a pointer to counters
// owned by the probe.
struct counts {
    int copies    = 0;
    int moves     = 0;
    int destroyed = 0;

    friend constexpr bool operator==(const counts&, const counts&) = default;
    friend std::ostream&  operator<<(std::ostream& os, const counts& c) {
        return os << "{copies=" << c.copies << ", moves=" << c.moves << ", destroyed=" << c.destroyed << "}";
    }
};

struct counted {
    counts* c;

    constexpr explicit counted(counts* c) noexcept : c(c) {}
    constexpr counted(const counted& other) noexcept : c(other.c) { ++c->copies; }
    constexpr counted(counted&& other) noexcept : c(other.c) { ++c->moves; }
    constexpr ~counted() { ++c->destroyed; }

    constexpr int operator()() const { return 7; }
};

// Reports the value category and constness it was invoked as.
enum class called_as { lvalue = 1, rvalue, const_lvalue, const_rvalue };

struct qualified {
    constexpr called_as operator()() & { return called_as::lvalue; }
    constexpr called_as operator()() && { return called_as::rvalue; }
    constexpr called_as operator()() const& { return called_as::const_lvalue; }
    constexpr called_as operator()() const&& { return called_as::const_rvalue; }
};

struct qualified_noexcept {
    constexpr called_as operator()() & noexcept { return called_as::lvalue; }
    constexpr called_as operator()() && noexcept { return called_as::rvalue; }
    constexpr called_as operator()() const& noexcept { return called_as::const_lvalue; }
    constexpr called_as operator()() const&& noexcept { return called_as::const_rvalue; }
};

struct accumulator {
    int           total = 0;
    constexpr int add(int x) { return total += x; }
};

constexpr int twice(int x) { return 2 * x; }

// Too big for the local buffer, so a runtime target is heap-allocated.
struct large {
    long          values[16] = {};
    constexpr int operator()(int x) const { return x + static_cast<int>(values[0]); }
};

// Small, but not nothrow-move-constructible, so it is never stored locally.
struct throwing_move {
    int value                 = 0;
    constexpr throwing_move() = default;
    constexpr explicit throwing_move(int v) : value(v) {}
    constexpr throwing_move(const throwing_move&) = default;
    constexpr throwing_move(throwing_move&& other) noexcept(false) : value(other.value) {}
    constexpr int operator()() const { return value; }
};

// Constructible from an initializer_list plus extra arguments.
struct summer {
    int total = 0;
    constexpr summer(std::initializer_list<int> values, int extra) : total(extra) {
        for (int v : values)
            total += v;
    }
    constexpr int operator()() const { return total; }
};

} // namespace

// ---------------------------------------------------------------------------
// [func.wrap.copy.class]
// ---------------------------------------------------------------------------

static_assert(std::is_same_v<copyable_function<int(long)>::result_type, int>);
static_assert(std::is_same_v<copyable_function<void() const && noexcept>::result_type, void>);

static_assert(std::is_copy_constructible_v<copyable_function<int()>>);
static_assert(std::is_copy_assignable_v<copyable_function<int()>>);
static_assert(std::is_nothrow_move_constructible_v<copyable_function<int()>>);
static_assert(std::is_nothrow_default_constructible_v<copyable_function<int()>>);
static_assert(std::is_nothrow_constructible_v<copyable_function<int()>, std::nullptr_t>);

// The call operator carries exactly the noexcept of the signature.
static_assert(!noexcept(std::declval<copyable_function<int()>&>()()));
static_assert(noexcept(std::declval<copyable_function<int() noexcept>&>()()));
static_assert(noexcept(std::declval<const copyable_function<int() const noexcept>&>()()));
static_assert(noexcept(std::declval<copyable_function<int() && noexcept>>()()));

// A constant-initialized empty wrapper is usable as a constexpr variable.
constexpr copyable_function<int(int)> constexpr_empty;
static_assert(!constexpr_empty);
static_assert(constexpr_empty == nullptr);

// ---------------------------------------------------------------------------
// [func.wrap.copy.ctor] is-callable-from
// ---------------------------------------------------------------------------

// cv and ref select which call operator of the target must be viable.
static_assert(std::is_constructible_v<copyable_function<int()>, int (*)()>);
static_assert(std::is_constructible_v<copyable_function<int() const>, int (*)()>);

namespace {
struct non_const_only {
    int operator()() { return 0; }
};
struct lvalue_only {
    int operator()() & { return 0; }
};
struct rvalue_only {
    int operator()() && { return 0; }
};
struct not_noexcept {
    int operator()() { return 0; }
};
} // namespace

static_assert(std::is_constructible_v<copyable_function<int()>, non_const_only>);
static_assert(!std::is_constructible_v<copyable_function<int() const>, non_const_only>);
static_assert(std::is_constructible_v<copyable_function<int() &>, lvalue_only>);
static_assert(!std::is_constructible_v<copyable_function<int() &&>, lvalue_only>);
static_assert(std::is_constructible_v<copyable_function<int() &&>, rvalue_only>);
static_assert(!std::is_constructible_v<copyable_function<int()>, rvalue_only>);
static_assert(!std::is_constructible_v<copyable_function<int() noexcept>, not_noexcept>);
// Return type must be convertible to R; any R accepts a call whose result is discarded.
static_assert(!std::is_constructible_v<copyable_function<int*()>, int (*)()>);
static_assert(std::is_constructible_v<copyable_function<void()>, int (*)()>);
static_assert(std::is_constructible_v<copyable_function<long()>, int (*)()>);

// ---------------------------------------------------------------------------
// [func.wrap.copy.ctor] default and nullptr constructors
// ---------------------------------------------------------------------------

TEST(CopyableFunction, DefaultConstructedHasNoTarget) {
    constexpr auto probe = [] {
        copyable_function<int()> f;
        return !f && f == nullptr;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, NullptrConstructedHasNoTarget) {
    constexpr auto probe = [] {
        copyable_function<int()> f(nullptr);
        return !f && f == nullptr;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

// ---------------------------------------------------------------------------
// [func.wrap.copy.ctor] move constructor
// ---------------------------------------------------------------------------

TEST(CopyableFunction, MoveConstructorTransfersTarget) {
    constexpr auto probe = [] {
        copyable_function<int(int)> f = [](int x) { return x + 1; };
        copyable_function<int(int)> g(std::move(f));
        return g(1);
    };
    EXPECT_EQ(constant_eval(probe), 2);
    EXPECT_EQ(probe(), 2);
}

TEST(CopyableFunction, MoveConstructorFromEmpty) {
    constexpr auto probe = [] {
        copyable_function<int()> f;
        copyable_function<int()> g(std::move(f));
        return !g;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, MoveConstructorMovesOnlyLocalTargets) {
    // A heap target is handed over, not moved: no special member runs.
    constexpr auto probe = [] {
        counts c;
        {
            copyable_function<int()> f(std::in_place_type<counted>, &c);
            copyable_function<int()> g(std::move(f));
            copyable_function<int()> h(std::move(g));
        }
        return c;
    };
    // Constant evaluation always uses the heap.
    EXPECT_EQ(constant_eval(probe), (counts{.copies = 0, .moves = 0, .destroyed = 1}));
    // At runtime `counted` fits the local buffer and is moved with the wrapper.
    EXPECT_EQ(probe(), (counts{.copies = 0, .moves = 2, .destroyed = 3}));
}

// ---------------------------------------------------------------------------
// [func.wrap.copy.ctor] converting constructor
// ---------------------------------------------------------------------------

TEST(CopyableFunction, ConstructFromLambda) {
    constexpr auto probe = [] {
        int                         k = 3;
        copyable_function<int(int)> f = [k](int x) { return x * k; };
        return f(5);
    };
    EXPECT_EQ(constant_eval(probe), 15);
    EXPECT_EQ(probe(), 15);
}

TEST(CopyableFunction, ConstructFromFunctionPointer) {
    constexpr auto probe = [] {
        copyable_function<int(int)> f = &twice;
        return f(4);
    };
    EXPECT_EQ(constant_eval(probe), 8);
    EXPECT_EQ(probe(), 8);
}

TEST(CopyableFunction, ConstructFromFunctionReference) {
    constexpr auto probe = [] {
        copyable_function<int(int)> f = twice;
        return f(4);
    };
    EXPECT_EQ(constant_eval(probe), 8);
    EXPECT_EQ(probe(), 8);
}

TEST(CopyableFunction, NullFunctionPointerGivesNoTarget) {
    constexpr auto probe = [] {
        int (*fp)(int) = nullptr;
        copyable_function<int(int)> f(fp);
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, NullMemberFunctionPointerGivesNoTarget) {
    constexpr auto probe = [] {
        int (accumulator::*pmf)(int) = nullptr;
        copyable_function<int(accumulator&, int)> f(pmf);
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, NullMemberDataPointerGivesNoTarget) {
    constexpr auto probe = [] {
        int accumulator::*                   pmd = nullptr;
        copyable_function<int(accumulator&)> f(pmd);
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, EmptyOtherSpecializationGivesNoTarget) {
    constexpr auto probe = [] {
        copyable_function<int() const> other;
        copyable_function<int()>       f(std::move(other));
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, NonEmptyOtherSpecializationIsWrapped) {
    constexpr auto probe = [] {
        copyable_function<int() const> other = [] { return 9; };
        copyable_function<int()>       f(std::move(other));
        return static_cast<bool>(f) && f() == 9;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, ConvertingConstructorMovesRvalue) {
    constexpr auto probe = [] {
        counts c;
        {
            counted                  source(&c);
            copyable_function<int()> f(std::move(source));
        }
        return c;
    };
    // One move into the target; both the source and the target are destroyed.
    EXPECT_EQ(constant_eval(probe), (counts{.copies = 0, .moves = 1, .destroyed = 2}));
    EXPECT_EQ(probe(), (counts{.copies = 0, .moves = 1, .destroyed = 2}));
}

TEST(CopyableFunction, ConvertingConstructorCopiesLvalue) {
    constexpr auto probe = [] {
        counts c;
        {
            counted                  source(&c);
            copyable_function<int()> f(source);
        }
        return c;
    };
    EXPECT_EQ(constant_eval(probe), (counts{.copies = 1, .moves = 0, .destroyed = 2}));
    EXPECT_EQ(probe(), (counts{.copies = 1, .moves = 0, .destroyed = 2}));
}

TEST(CopyableFunction, ConstructFromReferenceWrapper) {
    constexpr auto probe = [] {
        accumulator                 acc;
        auto                        add = [&acc](int x) { return acc.add(x); };
        copyable_function<int(int)> f   = std::ref(add);
        f(2);
        f(3);
        return acc.total;
    };
    EXPECT_EQ(constant_eval(probe), 5);
    EXPECT_EQ(probe(), 5);
}

// ---------------------------------------------------------------------------
// [func.wrap.copy.ctor] in-place constructors
// ---------------------------------------------------------------------------

TEST(CopyableFunction, InPlaceConstructsTargetFromArguments) {
    constexpr auto probe = [] {
        counts c;
        {
            copyable_function<int()> f(std::in_place_type<counted>, &c);
            if (f() != 7)
                c.copies = -1;
        }
        return c;
    };
    // Constructed directly: no copies or moves, one destruction.
    EXPECT_EQ(constant_eval(probe), (counts{.copies = 0, .moves = 0, .destroyed = 1}));
    EXPECT_EQ(probe(), (counts{.copies = 0, .moves = 0, .destroyed = 1}));
}

TEST(CopyableFunction, InPlaceWithInitializerList) {
    constexpr auto probe = [] {
        copyable_function<int()> f(std::in_place_type<summer>, {1, 2, 3}, 10);
        return f();
    };
    EXPECT_EQ(constant_eval(probe), 16);
    EXPECT_EQ(probe(), 16);
}

// ---------------------------------------------------------------------------
// [func.wrap.copy.ctor] assignment
// ---------------------------------------------------------------------------

TEST(CopyableFunction, MoveAssignmentReplacesTarget) {
    constexpr auto probe = [] {
        counts c;
        int    result = 0;
        {
            copyable_function<int()> f(std::in_place_type<counted>, &c);
            copyable_function<int()> g = [] { return 3; };
            f                          = std::move(g);
            result                     = f();
            // The old target is gone after the assignment: every counted
            // object created (the original plus each copy or move) has been
            // destroyed. At runtime the local target is moved by the swap
            // inside the assignment; on the heap it is not.
            if (c.destroyed != 1 + c.copies + c.moves)
                result = -1;
        }
        return result;
    };
    EXPECT_EQ(constant_eval(probe), 3);
    EXPECT_EQ(probe(), 3);
}

TEST(CopyableFunction, MoveAssignmentFromEmpty) {
    constexpr auto probe = [] {
        copyable_function<int()> f = [] { return 3; };
        copyable_function<int()> g;
        f = std::move(g);
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, SelfMoveAssignmentKeepsTarget) {
    constexpr auto probe = [] {
        copyable_function<int()>  f = [] { return 3; };
        copyable_function<int()>& r = f;
        f                           = std::move(r);
        return static_cast<bool>(f) && f() == 3;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, MoveAssignmentReturnsThis) {
    constexpr auto probe = [] {
        copyable_function<int()> f;
        copyable_function<int()> g = [] { return 3; };
        return &(f = std::move(g)) == &f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, NullptrAssignmentDestroysTarget) {
    constexpr auto probe = [] {
        counts                   c;
        copyable_function<int()> f(std::in_place_type<counted>, &c);
        bool                     returns_this = &(f = nullptr) == &f;
        return returns_this && !f && c.destroyed == 1;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, ConvertingAssignmentReplacesTarget) {
    constexpr auto probe = [] {
        copyable_function<int(int)> f            = [](int x) { return x; };
        bool                        returns_this = &(f = [](int x) { return x * 10; }) == &f;
        return returns_this ? f(2) : -1;
    };
    EXPECT_EQ(constant_eval(probe), 20);
    EXPECT_EQ(probe(), 20);
}

TEST(CopyableFunction, ConvertingAssignmentFromNullFunctionPointer) {
    constexpr auto probe = [] {
        copyable_function<int(int)> f = [](int x) { return x; };
        int (*fp)(int)                = nullptr;
        f                             = fp;
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

// ---------------------------------------------------------------------------
// [func.wrap.copy.ctor] destructor
// ---------------------------------------------------------------------------

TEST(CopyableFunction, DestructorDestroysTarget) {
    constexpr auto probe = [] {
        counts c;
        {
            copyable_function<int()> f(std::in_place_type<counted>, &c);
        }
        return c.destroyed;
    };
    EXPECT_EQ(constant_eval(probe), 1);
    EXPECT_EQ(probe(), 1);
}

// ---------------------------------------------------------------------------
// [func.wrap.copy.inv] invocation
// ---------------------------------------------------------------------------

TEST(CopyableFunction, OperatorBool) {
    constexpr auto probe = [] {
        copyable_function<int()> empty;
        copyable_function<int()> full = [] { return 0; };
        return !empty && static_cast<bool>(full);
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

// The target is invoked as `F inv-quals`: cv& when ref is empty, else cv ref.
TEST(CopyableFunction, InvokesTargetWithInvQuals) {
    constexpr auto probe = [] {
        copyable_function<called_as()>         plain(qualified{});
        copyable_function<called_as() &>       lref(qualified{});
        copyable_function<called_as() &&>      rref(qualified{});
        copyable_function<called_as() const>   c(qualified{});
        copyable_function<called_as() const&>  clref(qualified{});
        copyable_function<called_as() const&&> crref(qualified{});
        const auto&                            const_c     = c;
        const auto&                            const_clref = clref;
        return plain() == called_as::lvalue && lref() == called_as::lvalue && std::move(rref)() == called_as::rvalue &&
               const_c() == called_as::const_lvalue && const_clref() == called_as::const_lvalue &&
               std::move(std::as_const(crref))() == called_as::const_rvalue;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, InvokesTargetWithInvQualsNoexcept) {
    constexpr auto probe = [] {
        copyable_function<called_as() noexcept>          plain(qualified_noexcept{});
        copyable_function<called_as() & noexcept>        lref(qualified_noexcept{});
        copyable_function<called_as() && noexcept>       rref(qualified_noexcept{});
        copyable_function<called_as() const noexcept>    c(qualified_noexcept{});
        copyable_function<called_as() const & noexcept>  clref(qualified_noexcept{});
        copyable_function<called_as() const && noexcept> crref(qualified_noexcept{});
        const auto&                                      const_c     = c;
        const auto&                                      const_clref = clref;
        return plain() == called_as::lvalue && lref() == called_as::lvalue && std::move(rref)() == called_as::rvalue &&
               const_c() == called_as::const_lvalue && const_clref() == called_as::const_lvalue &&
               std::move(std::as_const(crref))() == called_as::const_rvalue;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

// gcc false positive at -O2 and above: calling through a member function
// pointer compiles to a branch on whether the pointer is virtual, and the
// virtual arm loads a vtable pointer from the object. Once the pointer has
// been through type-erased storage, gcc can't prove that arm dead, and warns
// that it reads 8 bytes of the 4-byte accumulator. libstdc++'s own
// std::move_only_function, std::copyable_function, and std::function draw the
// same warning for the same call.
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Warray-bounds"
#endif
TEST(CopyableFunction, InvokesMemberFunctionPointer) {
    constexpr auto probe = [] {
        accumulator                               acc;
        copyable_function<int(accumulator&, int)> f = &accumulator::add;
        f(acc, 4);
        return f(acc, 5);
    };
    EXPECT_EQ(constant_eval(probe), 9);
    EXPECT_EQ(probe(), 9);
}
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic pop
#endif

TEST(CopyableFunction, InvokesMemberDataPointer) {
    constexpr auto probe = [] {
        accumulator                                acc{.total = 6};
        copyable_function<int(const accumulator&)> f = &accumulator::total;
        return f(acc);
    };
    EXPECT_EQ(constant_eval(probe), 6);
    EXPECT_EQ(probe(), 6);
}

TEST(CopyableFunction, InvokeConvertsResultToR) {
    constexpr auto probe = [] {
        copyable_function<long(int)> f = &twice;
        return f(21);
    };
    EXPECT_EQ(constant_eval(probe), 42L);
    EXPECT_EQ(probe(), 42L);
}

TEST(CopyableFunction, InvokeDiscardsResultForVoid) {
    constexpr auto probe = [] {
        accumulator                  acc;
        copyable_function<void(int)> f = [&acc](int x) { return acc.add(x); };
        f(5);
        return acc.total;
    };
    EXPECT_EQ(constant_eval(probe), 5);
    EXPECT_EQ(probe(), 5);
}

TEST(CopyableFunction, ForwardsArguments) {
    constexpr auto probe = [] {
        copyable_function<int(std::unique_ptr<int>, int&)> f = [](std::unique_ptr<int> p, int& out) {
            out = *p;
            return *p + 1;
        };
        int out    = 0;
        int result = f(std::make_unique<int>(4), out);
        return result * 10 + out;
    };
    EXPECT_EQ(constant_eval(probe), 54);
    EXPECT_EQ(probe(), 54);
}

// ---------------------------------------------------------------------------
// [func.wrap.copy.util]
// ---------------------------------------------------------------------------

TEST(CopyableFunction, MemberSwapExchangesTargets) {
    constexpr auto probe = [] {
        copyable_function<int()> f = [] { return 1; };
        copyable_function<int()> g = [] { return 2; };
        f.swap(g);
        return f() * 10 + g();
    };
    EXPECT_EQ(constant_eval(probe), 21);
    EXPECT_EQ(probe(), 21);
}

TEST(CopyableFunction, FriendSwapExchangesTargets) {
    constexpr auto probe = [] {
        copyable_function<int()> f = [] { return 1; };
        copyable_function<int()> g;
        swap(f, g);
        return !f && g() == 1;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, SelfSwapKeepsTarget) {
    constexpr auto probe = [] {
        copyable_function<int()> f = [] { return 1; };
        f.swap(f);
        return static_cast<bool>(f) && f() == 1;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, EqualityWithNullptr) {
    constexpr auto probe = [] {
        copyable_function<int()> empty;
        copyable_function<int()> full = [] { return 0; };
        // operator!= and the reversed forms are synthesized from operator==.
        return empty == nullptr && nullptr == empty && full != nullptr && nullptr != full;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

// ---------------------------------------------------------------------------
// Runtime-only: storage paths and exceptions
// ---------------------------------------------------------------------------
// Constant evaluation always puts the target on the heap. These exercise the
// runtime choice between the local buffer and the heap.

TEST(CopyableFunction, RuntimeLargeTargetUsesHeap) {
    copyable_function<int(int)> f = large{{5}};
    copyable_function<int(int)> g = std::move(f);
    EXPECT_FALSE(f);
    EXPECT_EQ(g(1), 6);
    g = nullptr;
    EXPECT_FALSE(g);
}

TEST(CopyableFunction, RuntimeThrowingMoveTargetUsesHeap) {
    copyable_function<int()> f(std::in_place_type<throwing_move>, 8);
    copyable_function<int()> g = std::move(f);
    EXPECT_FALSE(f);
    EXPECT_EQ(g(), 8);
}

TEST(CopyableFunction, RuntimeSwapLocalWithHeap) {
    copyable_function<int(int)> local = [](int x) { return x; };
    copyable_function<int(int)> heap  = large{{100}};
    swap(local, heap);
    EXPECT_EQ(local(1), 101);
    EXPECT_EQ(heap(1), 1);
}

TEST(CopyableFunction, RuntimeTargetConstructionExceptionPropagates) {
    struct throws_on_copy {
        throws_on_copy() = default;
        throws_on_copy(const throws_on_copy&) { throw std::runtime_error("copy"); }
        int operator()() const { return 0; }
    };
    throws_on_copy source;
    EXPECT_THROW((copyable_function<int()>(source)), std::runtime_error);
}

TEST(CopyableFunction, RuntimeInvocationExceptionPropagates) {
    copyable_function<int()> f = []() -> int { throw std::runtime_error("call"); };
    EXPECT_THROW(f(), std::runtime_error);
}

// ---------------------------------------------------------------------------
// [func.wrap.copy.ctor] copy constructor and copy assignment
// ---------------------------------------------------------------------------

TEST(CopyableFunction, CopyConstructorCopiesTarget) {
    constexpr auto probe = [] {
        counts c;
        int    results = 0;
        {
            copyable_function<int()> f(std::in_place_type<counted>, &c);
            copyable_function<int()> g(f);
            results = f() + g();
        }
        return results == 14 ? c : counts{-1, -1, -1};
    };
    // Exactly one copy; the original and the copy are each destroyed once.
    EXPECT_EQ(constant_eval(probe), (counts{.copies = 1, .moves = 0, .destroyed = 2}));
    EXPECT_EQ(probe(), (counts{.copies = 1, .moves = 0, .destroyed = 2}));
}

TEST(CopyableFunction, CopyConstructorFromEmpty) {
    constexpr auto probe = [] {
        copyable_function<int()> f;
        copyable_function<int()> g(f);
        return !g && !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, CopiesHaveIndependentTargets) {
    constexpr auto probe = [] {
        copyable_function<int()> f = [n = 0]() mutable { return ++n; };
        f();                           // f's counter: 1
        copyable_function<int()> g(f); // g starts from a copy of 1
        f();
        f();
        int from_f = f(); // 4
        int from_g = g(); // 2
        return from_f * 10 + from_g;
    };
    EXPECT_EQ(constant_eval(probe), 42);
    EXPECT_EQ(probe(), 42);
}

TEST(CopyableFunction, CopyAssignmentReplacesTarget) {
    constexpr auto probe = [] {
        counts c;
        bool   ok = false;
        {
            copyable_function<int()> f = [] { return 3; };
            copyable_function<int()> g(std::in_place_type<counted>, &c);
            bool                     returns_this = &(f = g) == &f;
            ok                                    = returns_this && f() == 7 && g() == 7;
        }
        // Every counted object created (one in place, plus copies and moves)
        // has been destroyed.
        return ok && c.copies == 1 && c.destroyed == 1 + c.copies + c.moves;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, CopyAssignmentFromEmpty) {
    constexpr auto probe = [] {
        copyable_function<int()> f = [] { return 3; };
        copyable_function<int()> g;
        f = g;
        return !f && !g;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, SelfCopyAssignmentKeepsTarget) {
    constexpr auto probe = [] {
        copyable_function<int()>        f = [] { return 3; };
        const copyable_function<int()>& r = f;
        f                                 = r;
        return static_cast<bool>(f) && f() == 3;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(CopyableFunction, ConvertingConstructorFromOtherSpecializationLvalueCopies) {
    constexpr auto probe = [] {
        copyable_function<int() const> other = [] { return 9; };
        copyable_function<int()>       f(other);
        return static_cast<bool>(f) && static_cast<bool>(other) && f() == 9 && other() == 9;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

// ---------------------------------------------------------------------------
// Runtime-only: copying each storage path, and exceptions while copying
// ---------------------------------------------------------------------------

TEST(CopyableFunction, RuntimeCopyHeapTarget) {
    copyable_function<int(int)> f = large{{5}};
    copyable_function<int(int)> g(f);
    EXPECT_EQ(f(1), 6);
    EXPECT_EQ(g(2), 7);
}

TEST(CopyableFunction, RuntimeCopyThrowingMoveTarget) {
    copyable_function<int()> f(std::in_place_type<throwing_move>, 8);
    copyable_function<int()> g(f);
    EXPECT_EQ(f(), 8);
    EXPECT_EQ(g(), 8);
}

TEST(CopyableFunction, RuntimeCopyExceptionLeavesSourceIntact) {
    struct throws_on_copy {
        throws_on_copy() = default;
        throws_on_copy(const throws_on_copy&) { throw std::runtime_error("copy"); }
        int operator()() const { return 5; }
    };
    copyable_function<int()> f(std::in_place_type<throws_on_copy>);
    EXPECT_THROW((copyable_function<int()>(f)), std::runtime_error);
    EXPECT_EQ(f(), 5);

    copyable_function<int()> g = [] { return 1; };
    EXPECT_THROW(g = f, std::runtime_error);
    EXPECT_EQ(g(), 1); // copy-and-swap: the failed assignment changed nothing
}
