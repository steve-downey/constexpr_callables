// tests/beman/constexpr_callables/move_only_function.test.cpp       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/constexpr_callables/move_only_function.hpp>

#include <beman/constexpr_callables/move_only_function.hpp> // test 2nd include OK

#include <beman/constexpr_callables/copyable_function.hpp>

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
using beman::constexpr_callables::move_only_function;
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
// [func.wrap.move.class]
// ---------------------------------------------------------------------------

static_assert(std::is_same_v<move_only_function<int(long)>::result_type, int>);
static_assert(std::is_same_v<move_only_function<void() const && noexcept>::result_type, void>);

static_assert(!std::is_copy_constructible_v<move_only_function<int()>>);
static_assert(!std::is_copy_assignable_v<move_only_function<int()>>);
static_assert(std::is_nothrow_move_constructible_v<move_only_function<int()>>);
static_assert(std::is_nothrow_default_constructible_v<move_only_function<int()>>);
static_assert(std::is_nothrow_constructible_v<move_only_function<int()>, std::nullptr_t>);

// The call operator carries exactly the noexcept of the signature.
static_assert(!noexcept(std::declval<move_only_function<int()>&>()()));
static_assert(noexcept(std::declval<move_only_function<int() noexcept>&>()()));
static_assert(noexcept(std::declval<const move_only_function<int() const noexcept>&>()()));
static_assert(noexcept(std::declval<move_only_function<int() && noexcept>>()()));

// A constant-initialized empty wrapper is usable as a constexpr variable.
constexpr move_only_function<int(int)> constexpr_empty;
static_assert(!constexpr_empty);
static_assert(constexpr_empty == nullptr);

// ---------------------------------------------------------------------------
// [func.wrap.move.ctor] is-callable-from
// ---------------------------------------------------------------------------

// cv and ref select which call operator of the target must be viable.
static_assert(std::is_constructible_v<move_only_function<int()>, int (*)()>);
static_assert(std::is_constructible_v<move_only_function<int() const>, int (*)()>);

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

static_assert(std::is_constructible_v<move_only_function<int()>, non_const_only>);
static_assert(!std::is_constructible_v<move_only_function<int() const>, non_const_only>);
static_assert(std::is_constructible_v<move_only_function<int() &>, lvalue_only>);
static_assert(!std::is_constructible_v<move_only_function<int() &&>, lvalue_only>);
static_assert(std::is_constructible_v<move_only_function<int() &&>, rvalue_only>);
static_assert(!std::is_constructible_v<move_only_function<int()>, rvalue_only>);
static_assert(!std::is_constructible_v<move_only_function<int() noexcept>, not_noexcept>);
// Return type must be convertible to R; any R accepts a call whose result is discarded.
static_assert(!std::is_constructible_v<move_only_function<int*()>, int (*)()>);
static_assert(std::is_constructible_v<move_only_function<void()>, int (*)()>);
static_assert(std::is_constructible_v<move_only_function<long()>, int (*)()>);

// ---------------------------------------------------------------------------
// [func.wrap.move.ctor] default and nullptr constructors
// ---------------------------------------------------------------------------

TEST(MoveOnlyFunction, DefaultConstructedHasNoTarget) {
    constexpr auto probe = [] {
        move_only_function<int()> f;
        return !f && f == nullptr;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, NullptrConstructedHasNoTarget) {
    constexpr auto probe = [] {
        move_only_function<int()> f(nullptr);
        return !f && f == nullptr;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

// ---------------------------------------------------------------------------
// [func.wrap.move.ctor] move constructor
// ---------------------------------------------------------------------------

TEST(MoveOnlyFunction, MoveConstructorTransfersTarget) {
    constexpr auto probe = [] {
        move_only_function<int(int)> f = [](int x) { return x + 1; };
        move_only_function<int(int)> g(std::move(f));
        return g(1);
    };
    EXPECT_EQ(constant_eval(probe), 2);
    EXPECT_EQ(probe(), 2);
}

TEST(MoveOnlyFunction, MoveConstructorFromEmpty) {
    constexpr auto probe = [] {
        move_only_function<int()> f;
        move_only_function<int()> g(std::move(f));
        return !g;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, MoveConstructorMovesOnlyLocalTargets) {
    // A heap target is handed over, not moved: no special member runs.
    constexpr auto probe = [] {
        counts c;
        {
            move_only_function<int()> f(std::in_place_type<counted>, &c);
            move_only_function<int()> g(std::move(f));
            move_only_function<int()> h(std::move(g));
        }
        return c;
    };
    // Constant evaluation always uses the heap.
    EXPECT_EQ(constant_eval(probe), (counts{.copies = 0, .moves = 0, .destroyed = 1}));
    // At runtime `counted` fits the local buffer and is moved with the wrapper.
    EXPECT_EQ(probe(), (counts{.copies = 0, .moves = 2, .destroyed = 3}));
}

// ---------------------------------------------------------------------------
// [func.wrap.move.ctor] converting constructor
// ---------------------------------------------------------------------------

TEST(MoveOnlyFunction, ConstructFromLambda) {
    constexpr auto probe = [] {
        int                          k = 3;
        move_only_function<int(int)> f = [k](int x) { return x * k; };
        return f(5);
    };
    EXPECT_EQ(constant_eval(probe), 15);
    EXPECT_EQ(probe(), 15);
}

TEST(MoveOnlyFunction, ConstructFromFunctionPointer) {
    constexpr auto probe = [] {
        move_only_function<int(int)> f = &twice;
        return f(4);
    };
    EXPECT_EQ(constant_eval(probe), 8);
    EXPECT_EQ(probe(), 8);
}

TEST(MoveOnlyFunction, ConstructFromFunctionReference) {
    constexpr auto probe = [] {
        move_only_function<int(int)> f = twice;
        return f(4);
    };
    EXPECT_EQ(constant_eval(probe), 8);
    EXPECT_EQ(probe(), 8);
}

TEST(MoveOnlyFunction, NullFunctionPointerGivesNoTarget) {
    constexpr auto probe = [] {
        int (*fp)(int) = nullptr;
        move_only_function<int(int)> f(fp);
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, NullMemberFunctionPointerGivesNoTarget) {
    constexpr auto probe = [] {
        int (accumulator::*pmf)(int) = nullptr;
        move_only_function<int(accumulator&, int)> f(pmf);
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, NullMemberDataPointerGivesNoTarget) {
    constexpr auto probe = [] {
        int accumulator::*                    pmd = nullptr;
        move_only_function<int(accumulator&)> f(pmd);
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, EmptyOtherSpecializationGivesNoTarget) {
    constexpr auto probe = [] {
        move_only_function<int() const> other;
        move_only_function<int()>       f(std::move(other));
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, NonEmptyOtherSpecializationIsWrapped) {
    constexpr auto probe = [] {
        move_only_function<int() const> other = [] { return 9; };
        move_only_function<int()>       f(std::move(other));
        return static_cast<bool>(f) && f() == 9;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, EmptyCopyableFunctionGivesNoTarget) {
    constexpr auto probe = [] {
        copyable_function<int()>  empty;
        move_only_function<int()> from_lvalue(empty);
        move_only_function<int()> from_rvalue(std::move(empty));
        return !from_lvalue && !from_rvalue;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, NonEmptyCopyableFunctionIsWrapped) {
    constexpr auto probe = [] {
        copyable_function<int() const> other = [] { return 9; };
        move_only_function<int()>      f(other);
        return static_cast<bool>(f) && static_cast<bool>(other) && f() == 9;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, ConvertingConstructorMovesRvalue) {
    constexpr auto probe = [] {
        counts c;
        {
            counted                   source(&c);
            move_only_function<int()> f(std::move(source));
        }
        return c;
    };
    // One move into the target; both the source and the target are destroyed.
    EXPECT_EQ(constant_eval(probe), (counts{.copies = 0, .moves = 1, .destroyed = 2}));
    EXPECT_EQ(probe(), (counts{.copies = 0, .moves = 1, .destroyed = 2}));
}

TEST(MoveOnlyFunction, ConvertingConstructorCopiesLvalue) {
    constexpr auto probe = [] {
        counts c;
        {
            counted                   source(&c);
            move_only_function<int()> f(source);
        }
        return c;
    };
    EXPECT_EQ(constant_eval(probe), (counts{.copies = 1, .moves = 0, .destroyed = 2}));
    EXPECT_EQ(probe(), (counts{.copies = 1, .moves = 0, .destroyed = 2}));
}

TEST(MoveOnlyFunction, ConstructFromMoveOnlyTarget) {
    constexpr auto probe = [] {
        move_only_function<int()> f = [p = std::make_unique<int>(4)] { return *p; };
        return f();
    };
    EXPECT_EQ(constant_eval(probe), 4);
    EXPECT_EQ(probe(), 4);
}

TEST(MoveOnlyFunction, ConstructFromReferenceWrapper) {
    constexpr auto probe = [] {
        accumulator                  acc;
        auto                         add = [&acc](int x) { return acc.add(x); };
        move_only_function<int(int)> f   = std::ref(add);
        f(2);
        f(3);
        return acc.total;
    };
    EXPECT_EQ(constant_eval(probe), 5);
    EXPECT_EQ(probe(), 5);
}

// ---------------------------------------------------------------------------
// [func.wrap.move.ctor] in-place constructors
// ---------------------------------------------------------------------------

TEST(MoveOnlyFunction, InPlaceConstructsTargetFromArguments) {
    constexpr auto probe = [] {
        counts c;
        {
            move_only_function<int()> f(std::in_place_type<counted>, &c);
            if (f() != 7)
                c.copies = -1;
        }
        return c;
    };
    // Constructed directly: no copies or moves, one destruction.
    EXPECT_EQ(constant_eval(probe), (counts{.copies = 0, .moves = 0, .destroyed = 1}));
    EXPECT_EQ(probe(), (counts{.copies = 0, .moves = 0, .destroyed = 1}));
}

TEST(MoveOnlyFunction, InPlaceWithInitializerList) {
    constexpr auto probe = [] {
        move_only_function<int()> f(std::in_place_type<summer>, {1, 2, 3}, 10);
        return f();
    };
    EXPECT_EQ(constant_eval(probe), 16);
    EXPECT_EQ(probe(), 16);
}

// ---------------------------------------------------------------------------
// [func.wrap.move.ctor] assignment
// ---------------------------------------------------------------------------

TEST(MoveOnlyFunction, MoveAssignmentReplacesTarget) {
    constexpr auto probe = [] {
        counts c;
        int    result = 0;
        {
            move_only_function<int()> f(std::in_place_type<counted>, &c);
            move_only_function<int()> g = [] { return 3; };
            f                           = std::move(g);
            result                      = f();
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

TEST(MoveOnlyFunction, MoveAssignmentFromEmpty) {
    constexpr auto probe = [] {
        move_only_function<int()> f = [] { return 3; };
        move_only_function<int()> g;
        f = std::move(g);
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, SelfMoveAssignmentKeepsTarget) {
    constexpr auto probe = [] {
        move_only_function<int()>  f = [] { return 3; };
        move_only_function<int()>& r = f;
        f                            = std::move(r);
        return static_cast<bool>(f) && f() == 3;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, MoveAssignmentReturnsThis) {
    constexpr auto probe = [] {
        move_only_function<int()> f;
        move_only_function<int()> g = [] { return 3; };
        return &(f = std::move(g)) == &f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, NullptrAssignmentDestroysTarget) {
    constexpr auto probe = [] {
        counts                    c;
        move_only_function<int()> f(std::in_place_type<counted>, &c);
        bool                      returns_this = &(f = nullptr) == &f;
        return returns_this && !f && c.destroyed == 1;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, ConvertingAssignmentReplacesTarget) {
    constexpr auto probe = [] {
        move_only_function<int(int)> f            = [](int x) { return x; };
        bool                         returns_this = &(f = [](int x) { return x * 10; }) == &f;
        return returns_this ? f(2) : -1;
    };
    EXPECT_EQ(constant_eval(probe), 20);
    EXPECT_EQ(probe(), 20);
}

TEST(MoveOnlyFunction, ConvertingAssignmentFromNullFunctionPointer) {
    constexpr auto probe = [] {
        move_only_function<int(int)> f = [](int x) { return x; };
        int (*fp)(int)                 = nullptr;
        f                              = fp;
        return !f;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

// ---------------------------------------------------------------------------
// [func.wrap.move.ctor] destructor
// ---------------------------------------------------------------------------

TEST(MoveOnlyFunction, DestructorDestroysTarget) {
    constexpr auto probe = [] {
        counts c;
        {
            move_only_function<int()> f(std::in_place_type<counted>, &c);
        }
        return c.destroyed;
    };
    EXPECT_EQ(constant_eval(probe), 1);
    EXPECT_EQ(probe(), 1);
}

// ---------------------------------------------------------------------------
// [func.wrap.move.inv] invocation
// ---------------------------------------------------------------------------

TEST(MoveOnlyFunction, OperatorBool) {
    constexpr auto probe = [] {
        move_only_function<int()> empty;
        move_only_function<int()> full = [] { return 0; };
        return !empty && static_cast<bool>(full);
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

// The target is invoked as `F inv-quals`: cv& when ref is empty, else cv ref.
TEST(MoveOnlyFunction, InvokesTargetWithInvQuals) {
    constexpr auto probe = [] {
        move_only_function<called_as()>         plain(qualified{});
        move_only_function<called_as() &>       lref(qualified{});
        move_only_function<called_as() &&>      rref(qualified{});
        move_only_function<called_as() const>   c(qualified{});
        move_only_function<called_as() const&>  clref(qualified{});
        move_only_function<called_as() const&&> crref(qualified{});
        const auto&                             const_c     = c;
        const auto&                             const_clref = clref;
        return plain() == called_as::lvalue && lref() == called_as::lvalue && std::move(rref)() == called_as::rvalue &&
               const_c() == called_as::const_lvalue && const_clref() == called_as::const_lvalue &&
               std::move(std::as_const(crref))() == called_as::const_rvalue;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, InvokesTargetWithInvQualsNoexcept) {
    constexpr auto probe = [] {
        move_only_function<called_as() noexcept>          plain(qualified_noexcept{});
        move_only_function<called_as() & noexcept>        lref(qualified_noexcept{});
        move_only_function<called_as() && noexcept>       rref(qualified_noexcept{});
        move_only_function<called_as() const noexcept>    c(qualified_noexcept{});
        move_only_function<called_as() const & noexcept>  clref(qualified_noexcept{});
        move_only_function<called_as() const && noexcept> crref(qualified_noexcept{});
        const auto&                                       const_c     = c;
        const auto&                                       const_clref = clref;
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
// same warning for the same call. Drop the pragma once GCC PR 111750 is
// fixed: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111750
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Warray-bounds"
#endif
TEST(MoveOnlyFunction, InvokesMemberFunctionPointer) {
    constexpr auto probe = [] {
        accumulator                                acc;
        move_only_function<int(accumulator&, int)> f = &accumulator::add;
        f(acc, 4);
        return f(acc, 5);
    };
    EXPECT_EQ(constant_eval(probe), 9);
    EXPECT_EQ(probe(), 9);
}
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic pop
#endif

TEST(MoveOnlyFunction, InvokesMemberDataPointer) {
    constexpr auto probe = [] {
        accumulator                                 acc{.total = 6};
        move_only_function<int(const accumulator&)> f = &accumulator::total;
        return f(acc);
    };
    EXPECT_EQ(constant_eval(probe), 6);
    EXPECT_EQ(probe(), 6);
}

TEST(MoveOnlyFunction, InvokeConvertsResultToR) {
    constexpr auto probe = [] {
        move_only_function<long(int)> f = &twice;
        return f(21);
    };
    EXPECT_EQ(constant_eval(probe), 42L);
    EXPECT_EQ(probe(), 42L);
}

TEST(MoveOnlyFunction, InvokeDiscardsResultForVoid) {
    constexpr auto probe = [] {
        accumulator                   acc;
        move_only_function<void(int)> f = [&acc](int x) { return acc.add(x); };
        f(5);
        return acc.total;
    };
    EXPECT_EQ(constant_eval(probe), 5);
    EXPECT_EQ(probe(), 5);
}

TEST(MoveOnlyFunction, ForwardsArguments) {
    constexpr auto probe = [] {
        move_only_function<int(std::unique_ptr<int>, int&)> f = [](std::unique_ptr<int> p, int& out) {
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
// [func.wrap.move.util]
// ---------------------------------------------------------------------------

TEST(MoveOnlyFunction, MemberSwapExchangesTargets) {
    constexpr auto probe = [] {
        move_only_function<int()> f = [] { return 1; };
        move_only_function<int()> g = [] { return 2; };
        f.swap(g);
        return f() * 10 + g();
    };
    EXPECT_EQ(constant_eval(probe), 21);
    EXPECT_EQ(probe(), 21);
}

TEST(MoveOnlyFunction, FriendSwapExchangesTargets) {
    constexpr auto probe = [] {
        move_only_function<int()> f = [] { return 1; };
        move_only_function<int()> g;
        swap(f, g);
        return !f && g() == 1;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, SelfSwapKeepsTarget) {
    constexpr auto probe = [] {
        move_only_function<int()> f = [] { return 1; };
        f.swap(f);
        return static_cast<bool>(f) && f() == 1;
    };
    EXPECT_TRUE(constant_eval(probe));
    EXPECT_TRUE(probe());
}

TEST(MoveOnlyFunction, EqualityWithNullptr) {
    constexpr auto probe = [] {
        move_only_function<int()> empty;
        move_only_function<int()> full = [] { return 0; };
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

TEST(MoveOnlyFunction, RuntimeLargeTargetUsesHeap) {
    move_only_function<int(int)> f = large{{5}};
    move_only_function<int(int)> g = std::move(f);
    EXPECT_FALSE(f);
    EXPECT_EQ(g(1), 6);
    g = nullptr;
    EXPECT_FALSE(g);
}

TEST(MoveOnlyFunction, RuntimeThrowingMoveTargetUsesHeap) {
    move_only_function<int()> f(std::in_place_type<throwing_move>, 8);
    move_only_function<int()> g = std::move(f);
    EXPECT_FALSE(f);
    EXPECT_EQ(g(), 8);
}

TEST(MoveOnlyFunction, RuntimeSwapLocalWithHeap) {
    move_only_function<int(int)> local = [](int x) { return x; };
    move_only_function<int(int)> heap  = large{{100}};
    swap(local, heap);
    EXPECT_EQ(local(1), 101);
    EXPECT_EQ(heap(1), 1);
}

TEST(MoveOnlyFunction, RuntimeTargetConstructionExceptionPropagates) {
    struct throws_on_copy {
        throws_on_copy() = default;
        throws_on_copy(const throws_on_copy&) { throw std::runtime_error("copy"); }
        int operator()() const { return 0; }
    };
    throws_on_copy source;
    EXPECT_THROW((move_only_function<int()>(source)), std::runtime_error);
}

TEST(MoveOnlyFunction, RuntimeInvocationExceptionPropagates) {
    move_only_function<int()> f = []() -> int { throw std::runtime_error("call"); };
    EXPECT_THROW(f(), std::runtime_error);
}
