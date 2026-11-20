# papers/P3574/wording.mk                                        -*-makefile-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# ---- Generated [func.wrap.move], [func.wrap.copy], [func.wrap.ref] wording --
#
# The three annotated headers are one specgen document each and one paper: a
# single invocation renders them together, so --validate sees the paper-wide
# union of documented names. Included by the repository-root Makefile; paths
# are relative to the repository root.
#
# specgen writes one fragment per top-level clause; this makefile assembles
# wording.tex from them in the draft's order, after the hand-written
# preamble.tex. GNU Make 4.3 or newer, for the grouped target.

# specgen, from the `wording` dependency group in pyproject.toml: `uv run`
# installs that group into the project's venv on first use, at the pinned
# version. To try another specgen -- a newer release, or a development build --
# override it, e.g. SPECGEN=specgen for whatever is on PATH, or
# SPECGEN='uv run --with beman-specgen==X.Y.Z specgen'.
SPECGEN ?= $(UV) run --group wording specgen

# The C++ standard library specgen's embedded Clang parses against. function_ref
# needs std::constant_wrapper, so default to the GCC 16 installation that
# g++-16 belongs to; override with SPECGEN_GCC_TOOLCHAIN=<prefix>.
SPECGEN_GCC_TOOLCHAIN ?= $(patsubst %/bin/,%,$(dir $(realpath $(shell command -v g++-16 2> /dev/null))))

# The C++ working draft's LaTeX source, which the headers' `\asdraft` takes the
# unchanged wording from: a checkout of github.com/cplusplus/draft, named by its
# `source` directory. specgen records no revision, so the revision the wording
# is written against is WORDING_DRAFT_COMMIT here, and a checkout at another
# one is warned about and not refused: rebasing the paper onto a newer draft is
# a deliberate act, and the diff then shows what moved.
WORDING_DRAFT_SOURCE ?= ../draft/source
# 45b03900 is N5054, the last published working draft.
WORDING_DRAFT_COMMIT := 45b03900

# Whether the draft is there to read, and a warning when it is not the revision
# the wording is written against.
define wording_draft_present
test -f $(WORDING_DRAFT_SOURCE)/utilities.tex
endef
define wording_draft_revision_check
rev=$$(git -C $(WORDING_DRAFT_SOURCE) rev-parse --short=8 HEAD 2> /dev/null); \
if [ -n "$$rev" ] && [ "$$rev" != "$(WORDING_DRAFT_COMMIT)" ]; then \
    echo "warning: $(WORDING_DRAFT_SOURCE) is at $$rev; the wording is written against $(WORDING_DRAFT_COMMIT)" >&2; \
fi
endef

WORDING_DIR := papers/P3574/wording

WORDING_HEADERS := $(addprefix include/beman/constexpr_callables/, \
    move_only_function.hpp copyable_function.hpp function_ref.hpp)

# In the draft's clause order, by stable name. specgen also writes one
# *.root.tex per document, holding what sits outside every clause; for these
# headers that is nothing, and the files are not part of the wording.
WORDING_CLAUSES := func.wrap.move func.wrap.copy func.wrap.ref

WORDING_FRAGMENTS := \
    $(addprefix $(WORDING_DIR)/fragments/,$(addsuffix .tex,$(WORDING_CLAUSES)))

# Where a run's standard error is kept for the findings check below.
WORDING_FINDINGS := $(BUILD_DIR)/wording.findings

# specgen writes here first; the fragments are copied into $(WORDING_DIR) only
# when the run reports no findings. Writing in place would leave rejected
# output newer than the headers, so make would take it as up to date.
WORDING_STAGE := $(BUILD_DIR)/wording-stage

SPECGEN_CLANG_ARGS := -std=c++2c -Iinclude
ifneq ($(SPECGEN_GCC_TOOLCHAIN),)
SPECGEN_CLANG_ARGS += --gcc-toolchain=$(SPECGEN_GCC_TOOLCHAIN)
endif

# One specgen run over the three headers, splitting into $(1). Any further
# options, $(2), follow the fixed ones. Each document gets an explicit root
# name: the one specgen derives is the document's top clause's own stable
# name, which would collide with that clause's fragment.
#
# --validate prints notes as well as warnings and errors. The notes are
# informational (helpers that are \omit'ted and named by no wording), so the
# run fails on any warning or error, and passes notes through.
#
# --warnings-as-errors fails the run on a validation or docblock warning, but
# not on one from reading the draft: the importer's (a construct kept as raw
# LaTeX) and \asdraft's resolution are printed and the run still exits 0
# (specgen #372). So standard error is checked as well, and a warning from
# anywhere fails.
define specgen_wording
mkdir -p $(dir $(WORDING_FINDINGS)); \
$(SPECGEN) generate $(WORDING_HEADERS) \
    --backend latex --validate --warnings-as-errors \
    --split $(1) \
    --root func.wrap.move.root \
    --root func.wrap.copy.root \
    --root func.wrap.ref.root \
    $(2) \
    --no-compile-commands -- $(SPECGEN_CLANG_ARGS) \
    > /dev/null 2> $(WORDING_FINDINGS); \
status=$$?; \
cat $(WORDING_FINDINGS) >&2; \
if [ $$status -ne 0 ] || grep -q -E ': (warning|error):' $(WORDING_FINDINGS); then \
    echo "error: specgen reported findings; fix the header markup" >&2; \
    exit 1; \
fi
endef

# The fragments of $(1)/fragments assembled, in clause order, after the
# preamble $(2) (preamble.tex if empty), into $(1)/wording.tex. Written to a
# temporary first, so that a half-written file is never mistaken for a
# finished one.
define assemble_wording
cat $(or $(2),$(WORDING_DIR)/preamble.tex) > $(1)/wording.tex.tmp
for clause in $(WORDING_CLAUSES); do \
    printf '\n' >> $(1)/wording.tex.tmp; \
    cat $(1)/fragments/$$clause.tex >> $(1)/wording.tex.tmp; \
done
mv $(1)/wording.tex.tmp $(1)/wording.tex
endef

# A grouped target: one invocation writes all of these, and make must not run
# it once per fragment. Without specgen the checked-in fragments are used as
# they are, with a warning, so the paper still builds. Whether specgen is there
# is asked by running it, since $(SPECGEN) is a command line and not a name.
$(WORDING_FRAGMENTS) &: $(WORDING_HEADERS) papers/P3574/wording.mk
	@if $(SPECGEN) --version > /dev/null 2>&1 && $(wording_draft_present); then \
	    $(wording_draft_revision_check); \
	    rm -rf $(WORDING_STAGE) && \
	    mkdir -p papers/.deps $(WORDING_STAGE) $(WORDING_DIR)/fragments && \
	    $(call specgen_wording,$(WORDING_STAGE), \
	        --inherit-draft $(WORDING_DRAFT_SOURCE) \
	        --depfile papers/.deps/wording.d \
	        $(addprefix --dep-target ,$(WORDING_FRAGMENTS))) && \
	    for clause in $(WORDING_CLAUSES); do \
	        cp $(WORDING_STAGE)/$$clause.tex $(WORDING_DIR)/fragments/$$clause.tex; \
	    done; \
	else \
	    echo "warning: '$(SPECGEN)' or the draft's source ($(WORDING_DRAFT_SOURCE)) not found; using the checked-in wording" >&2; \
	fi

$(WORDING_DIR)/wording.tex: $(WORDING_DIR)/preamble.tex $(WORDING_FRAGMENTS)
	@$(call assemble_wording,$(WORDING_DIR))
	@echo "Wrote $@"

# $(wildcard) so that, before the first run, make does not try to remake the
# depfile (the root Makefile's .DEFAULT rule would hand it to CMake).
-include $(wildcard papers/.deps/wording.d)

.PHONY: wording
wording: $(WORDING_DIR)/wording.tex ## Regenerate papers/P3574/wording/ from the annotated headers via specgen

# Regenerates into a scratch directory and compares, so the checked-in wording
# is never touched.
.PHONY: wording-check
wording-check: ## Check the checked-in wording matches what the headers generate
	@$(wording_draft_present) || { echo "error: the draft's source is not at $(WORDING_DRAFT_SOURCE); set WORDING_DRAFT_SOURCE" >&2; exit 1; }
	@$(wording_draft_revision_check)
	rm -rf $(BUILD_DIR)/wording-check
	mkdir -p $(BUILD_DIR)/wording-check/fragments
	@$(call specgen_wording,$(BUILD_DIR)/wording-check/fragments, \
	    --inherit-draft $(WORDING_DRAFT_SOURCE))
	@$(call assemble_wording,$(BUILD_DIR)/wording-check)
	diff $(WORDING_DIR)/wording.tex $(BUILD_DIR)/wording-check/wording.tex
	for fragment in $(notdir $(WORDING_FRAGMENTS)); do \
	    diff $(WORDING_DIR)/fragments/$$fragment $(BUILD_DIR)/wording-check/fragments/$$fragment || exit 1; \
	done

# ---- The wording as a diff against the working draft ----------------------
#
# The same wording, marked with \added and \removed against the working draft
# itself: specgen reads the three clauses from the draft's LaTeX, by stable
# name, and diffs the headers' wording against them. What a header does not
# change is the draft's own text, inherited with `\asdraft`, so the diff is
# what P3574 changes and nothing else.
#
# WORDING_DIFF_CONTEXT is specgen's --diff-context: changed (whole changed
# subclauses), items (unchanged runs of items elided as [...]), decls (an
# unchanged description inside a changed item elided too, which leaves little
# but the declarations that gain constexpr), or all.
#
# The draft is not a file make can date, so the target always runs.

WORDING_DIFF_CONTEXT ?= changed
WORDING_DIFF_DIR := $(WORDING_DIR)/diff
WORDING_DIFF_STAGE := $(BUILD_DIR)/wording-diff-stage

.PHONY: wording-diff
wording-diff: ## Regenerate papers/P3574/wording/diff/, the wording marked against the working draft
	@$(wording_draft_present) || { echo "error: the draft's source is not at $(WORDING_DRAFT_SOURCE); set WORDING_DRAFT_SOURCE" >&2; exit 1; }
	@$(wording_draft_revision_check)
	@rm -rf $(WORDING_DIFF_STAGE)
	@mkdir -p $(WORDING_DIFF_STAGE) $(WORDING_DIFF_DIR)/fragments
	@$(call specgen_wording,$(WORDING_DIFF_STAGE), \
	    --baseline-draft $(WORDING_DRAFT_SOURCE) --diff-context $(WORDING_DIFF_CONTEXT))
	@for clause in $(WORDING_CLAUSES); do \
	    cp $(WORDING_DIFF_STAGE)/$$clause.tex $(WORDING_DIFF_DIR)/fragments/$$clause.tex; \
	done
	@$(call assemble_wording,$(WORDING_DIFF_DIR),$(WORDING_DIR)/diff-preamble.tex)
	@echo "Wrote $(WORDING_DIFF_DIR)/wording.tex"

# ---- The wording as an edit of the draft's own source ----------------------
#
# The paper's change made in the draft's LaTeX itself, as a patch that `git
# apply` takes in a checkout of cplusplus/draft: specgen's --apply-to-draft
# rewrites only the tokens the paper changes, so the draft's line breaks, index
# entries and comments are left as they are. This is what a pull request to the
# draft would contain. A change specgen cannot place in the draft's source yet
# (specgen #346) fails the run.
#
# Like the wording diff, it reads the draft, so the target always runs.

WORDING_DRAFT_PATCH := $(WORDING_DIR)/draft.patch
WORDING_DRAFT_STAGE := $(BUILD_DIR)/wording-draft-stage

.PHONY: wording-draft-patch
wording-draft-patch: ## Regenerate papers/P3574/wording/draft.patch, the change as a patch to the draft's source
	@$(wording_draft_present) || { echo "error: the draft's source is not at $(WORDING_DRAFT_SOURCE); set WORDING_DRAFT_SOURCE" >&2; exit 1; }
	@$(wording_draft_revision_check)
	@rm -rf $(WORDING_DRAFT_STAGE)
	@mkdir -p $(WORDING_DRAFT_STAGE) $(dir $(WORDING_FINDINGS))
	@$(SPECGEN) generate $(WORDING_HEADERS) \
	    --apply-to-draft $(WORDING_DRAFT_SOURCE) -o $(WORDING_DRAFT_STAGE) \
	    --no-compile-commands -- $(SPECGEN_CLANG_ARGS) \
	    2> $(WORDING_FINDINGS); \
	status=$$?; \
	cat $(WORDING_FINDINGS) >&2; \
	if [ $$status -ne 0 ] || grep -q -E ': (warning|error):' $(WORDING_FINDINGS); then \
	    echo "error: specgen could not apply the wording to the draft's source" >&2; \
	    exit 1; \
	fi
	@: > $(WORDING_DRAFT_PATCH).tmp
	@for file in $$(cd $(WORDING_DRAFT_STAGE) && ls); do \
	    diff -u --label a/source/$$file --label b/source/$$file \
	        $(WORDING_DRAFT_SOURCE)/$$file $(WORDING_DRAFT_STAGE)/$$file >> $(WORDING_DRAFT_PATCH).tmp; \
	    [ $$? -le 1 ] || exit 1; \
	done
	@mv $(WORDING_DRAFT_PATCH).tmp $(WORDING_DRAFT_PATCH)
	@echo "Wrote $(WORDING_DRAFT_PATCH)"
