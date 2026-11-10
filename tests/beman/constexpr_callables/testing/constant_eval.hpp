// tests/beman/constexpr_callables/testing/constant_eval.hpp          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Reporting compile-time facts through the runtime test framework.
//
// The usual way to test a constexpr contract is `static_assert`. That has one
// bad property as a *test*: a wrong answer is a translation failure, so the
// only thing anyone ever sees is a compiler diagnostic. The test run reports
// nothing, and because the build stops at the first failing assertion no other
// test in the file is exercised at all.
//
// `constant_eval` separates the two questions a constexpr test actually asks:
//
//   1. "can this be constant-evaluated at all?" — still a hard translation
//      failure, because that is a property of the code, not of a value.
//   2. "does it produce the right answer?" — an ordinary runtime comparison
//      inside a TEST, so a wrong answer is *reported*, with the actual and
//      expected values, alongside every other test in the run.
//
// `constant_eval` is `consteval`, so a call to it is an immediate invocation:
// the probe is evaluated during translation and its result is required to be
// a constant expression. The result then behaves as an ordinary prvalue, free
// to be compared by EXPECT_EQ.
//
// A probe is a plain lambda, not a `consteval` one, so the same probe body is
// run in both evaluation modes. The wrappers take different paths in the two
// modes — a target is always heap-allocated during constant evaluation, and
// may live in the local buffer at runtime — so every constant-evaluated check
// is paired with the same probe run at runtime:
//
//   TEST(MoveOnlyFunction, CallsTarget) {
//       constexpr auto probe = [] {
//           move_only_function<int(int)> f = [](int x) { return x + 1; };
//           return f(1);
//       };
//       EXPECT_EQ(constant_eval(probe), 2); // constant evaluation
//       EXPECT_EQ(probe(), 2);              // ordinary evaluation
//   }
//
// Two constraints on a probe follow from the above:
//
//   - It takes no arguments and captures nothing. A closure over a runtime
//     value is not a constant expression. Construct whatever the probe
//     observes inside the probe.
//   - It returns a literal type by value — a scalar, or a small aggregate of
//     scalars — never a reference to something it created, and never a type
//     whose value cannot escape constant evaluation. A wrapper holding a
//     target cannot escape either: its allocation must be freed during the
//     evaluation. Reduce such state to scalars inside the probe.
//
// Give a returned aggregate an `operator==` and a `PrintTo` or `operator<<`,
// so that a mismatch prints both states.

#ifndef BEMAN_CONSTEXPR_CALLABLES_TESTING_CONSTANT_EVAL_HPP
#define BEMAN_CONSTEXPR_CALLABLES_TESTING_CONSTANT_EVAL_HPP

namespace beman::constexpr_callables::testing {

// Constant-evaluate `probe` and return its result as an ordinary value.
template <class Probe>
consteval auto constant_eval(Probe probe) {
    return probe();
}

} // namespace beman::constexpr_callables::testing

#endif // BEMAN_CONSTEXPR_CALLABLES_TESTING_CONSTANT_EVAL_HPP
