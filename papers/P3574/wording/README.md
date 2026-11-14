# Generated P3574 wording

Everything in this directory except `preamble.tex` and this file is generated
from the `//!` docblocks in
`include/beman/constexpr_callables/{move_only_function,copyable_function,function_ref}.hpp`
by [specgen](https://github.com/steve-downey/specgen). Don't hand-edit it: the
header comments are the source of truth. `preamble.tex` is the comment block at
the top of `wording.tex`, and is written by hand.

```sh
make wording        # regenerate, if a header changed
make wording-check  # regenerate into .build/ and compare with what's checked in
```

`make wording` needs `specgen` on `PATH` (or `make SPECGEN=/path/to/specgen
wording`), a checkout of [the C++ working draft](https://github.com/cplusplus/draft)
(below), and GNU Make 4.3 or newer: one specgen run writes all three
fragments, which the makefile states as a grouped target (`&:`). Without
specgen, or without the draft, the checked-in wording is used as it is, with a
warning.

## The draft

Most of the wording is the working draft's, unchanged: the headers' `//!`
docblocks say `\asdraft` where the draft's text for a subclause, a class, or a
member is the paper's, and specgen reads it from the draft's LaTeX rather than
having it retyped here. Only what P3574 changes is written in the headers: the
declarations, with `constexpr`, and the few places the wording is authored
(below). `make wording` and `make wording-diff` read the draft from
`WORDING_DRAFT_SOURCE`, the `source` directory of a checkout, `../draft/source`
unless set:

```sh
make WORDING_DRAFT_SOURCE=~/src/cplusplus/draft/source wording
```

specgen records no draft revision. The wording is written against the draft
commit `WORDING_DRAFT_COMMIT` in `papers/P3574/wording.mk`, and a checkout at
another one is warned about. Moving the paper to a newer draft is a deliberate
act: update the checkout and the commit, and the diff shows what moved.

The rules are in `papers/P3574/wording.mk`, which the repository-root
`Makefile` includes. specgen writes what it read to `papers/.deps/wording.d`,
which lists every header each parse touched, `detail/` headers included, so
editing a docblock regenerates the wording and editing nothing regenerates
nothing. A run that reports a warning or an error fails, and leaves the
checked-in fragments as they were. Notes, such as an `\omit`ted helper named
by no wording, are printed and allowed.

specgen's embedded Clang parses the headers against GCC 16's libstdc++, because
`function_ref` needs `std::constant_wrapper`. The makefile finds the
installation that `g++-16` belongs to; set `SPECGEN_GCC_TOOLCHAIN` to another
GCC prefix to override it.

## `wording.tex`

`preamble.tex`, then the generated subclauses in the draft's order:
[func.wrap.move], [func.wrap.copy], [func.wrap.ref]. Each is a `\rSec3` with
its `\rSec4` subclauses, so this is what replaces those three subclauses
inside `\rSec2[func.wrap]` in [the draft](https://github.com/cplusplus/draft)'s
`source/utilities.tex`.

The wording is the current working draft's with `constexpr` added to the
owning wrappers' interfaces, as P3574 proposes, plus one addition the paper
doesn't yet state: `function_ref`'s call operator is `constexpr` too.
`function_ref(F*)` stays non-`constexpr`, as in the standard.

## `fragments/`

The same subclauses, one file per `\rSec3`, named for its stable name, for
`\input` into a paper's own `[func.wrap]`.

## `diff/`

The same wording marked against the working draft, with `\added` and
`\removed`, as the paper presents it: `diff/wording.tex` (after
`diff-preamble.tex`) and `diff/fragments/`. Regenerate it with

```sh
make wording-diff                              # whole changed subclauses
make wording-diff WORDING_DIFF_CONTEXT=items   # unchanged items elided as [...]
```

The baseline is the working draft itself: specgen reads the three clauses from
the draft's LaTeX, by stable name, and diffs the headers' wording against them
(`--baseline-draft`). Since the draft's own text is what the headers inherit,
the diff is exactly what P3574 changes.

The old baseline, the branch `p3574-baseline` and the tag
`p3574-baseline-draft`, was these headers with `constexpr` removed, kept so
specgen could diff against a revision. Nothing reads it now, and it can be
deleted.

The target always runs: the draft isn't a file make can date.

## How the headers carry the wording

- Each owning wrapper is one partial specialization standing for all twelve,
  so its class synopsis is authored with `\verbatim-synopsis` in the draft's
  placeholder form, `constexpr` written in. It can't be `\asdraft-synopsis`
  because `operator()` is declared by `detail::call_base`, not by the class, so
  there is no declaration of it for specgen to patch the draft's synopsis from.
  specgen compares the authored synopsis with the class's own declarations under
  `--validate` and notes the one declaration the class does not make.
- The general subclauses, the class descriptions, and every member's
  description are `//! \asdraft`, and take the draft's text. `operator()` is
  declared by `detail::call_base`, so its item is a detached
  `\verbatim-itemdecl` with `\asdraft` before it: the declaration, with
  `constexpr`, is written here, and the description is the draft's.
  *is-callable-from* is authored, because the draft's index has no entry for an
  exposition-only variable to pair it by.
- Both `function_ref` specializations are defined in
  `detail/function_ref_impl.hpp`, outside the document, so its synopsis and
  every item's declaration are authored, detached, at the end of
  `function_ref.hpp`, each with `\asdraft` taking the description. specgen
  still validates their spans and leakage.
- specgen reports, as warnings, the draft macros it has no inline kind for
  yet (`\ref`, `\cvqual`, `\exposconcept`, `\newline`, `\opt`) in
  [func.wrap.ref], and writes them back as they were. `wording.mk` lets exactly
  those through; anything else it reports fails the run.
