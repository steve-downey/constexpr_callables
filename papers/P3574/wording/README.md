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
wording`) and GNU Make 4.3 or newer: one specgen run writes all three
fragments, which the makefile states as a grouped target (`&:`). Without
specgen the checked-in wording is used as it is, with a warning.

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

## How the headers carry the wording

- Each owning wrapper is one partial specialization standing for all twelve,
  so its class synopsis is authored with `\verbatim-synopsis` in the draft's
  placeholder form. Its members' items are extracted from the code, with their
  descriptions in docblocks. `operator()` is declared by `detail::call_base`,
  so its item is a detached `\verbatim-itemdecl`.
- Both `function_ref` specializations are defined in
  `detail/function_ref_impl.hpp`, outside the document, so its synopsis and
  every item are authored, detached, at the end of `function_ref.hpp`. specgen
  still validates their spans and leakage.
- Prose that the draft writes unlabelled (the general subclauses, the class
  descriptions, *is-callable-from*'s description) is unlabelled prose in the
  docblocks.
