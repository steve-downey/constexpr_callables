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

SPECGEN ?= specgen

# The C++ standard library specgen's embedded Clang parses against. function_ref
# needs std::constant_wrapper, so default to the GCC 16 installation that
# g++-16 belongs to; override with SPECGEN_GCC_TOOLCHAIN=<prefix>.
SPECGEN_GCC_TOOLCHAIN ?= $(patsubst %/bin/,%,$(dir $(realpath $(shell command -v g++-16 2> /dev/null))))

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
# generate reports docblock findings on standard error and still exits 0, and
# --validate prints notes as well as warnings and errors. The notes are
# informational (helpers that are \omit'ted and named by no wording), so the
# run fails on any warning or error, and passes notes through.
define specgen_wording
mkdir -p $(dir $(WORDING_FINDINGS)); \
$(SPECGEN) generate $(WORDING_HEADERS) \
    --backend latex --validate \
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
# they are, with a warning, so the paper still builds.
$(WORDING_FRAGMENTS) &: $(WORDING_HEADERS) papers/P3574/wording.mk
	@if command -v $(SPECGEN) > /dev/null 2>&1; then \
	    rm -rf $(WORDING_STAGE) && \
	    mkdir -p papers/.deps $(WORDING_STAGE) $(WORDING_DIR)/fragments && \
	    $(call specgen_wording,$(WORDING_STAGE), \
	        --depfile papers/.deps/wording.d \
	        $(addprefix --dep-target ,$(WORDING_FRAGMENTS))) && \
	    for clause in $(WORDING_CLAUSES); do \
	        cp $(WORDING_STAGE)/$$clause.tex $(WORDING_DIR)/fragments/$$clause.tex; \
	    done; \
	else \
	    echo "warning: '$(SPECGEN)' not found; using the checked-in wording" >&2; \
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
	rm -rf $(BUILD_DIR)/wording-check
	mkdir -p $(BUILD_DIR)/wording-check/fragments
	@$(call specgen_wording,$(BUILD_DIR)/wording-check/fragments)
	@$(call assemble_wording,$(BUILD_DIR)/wording-check)
	diff $(WORDING_DIR)/wording.tex $(BUILD_DIR)/wording-check/wording.tex
	for fragment in $(notdir $(WORDING_FRAGMENTS)); do \
	    diff $(WORDING_DIR)/fragments/$$fragment $(BUILD_DIR)/wording-check/fragments/$$fragment || exit 1; \
	done

# ---- The wording as a diff against the working draft ----------------------
#
# The same wording, marked with \added and \removed against a baseline: a git
# revision of this repository whose headers carry the wording P3574 modifies.
# The tag p3574-baseline-draft is the working draft's [func.wrap.move],
# [func.wrap.copy], and [func.wrap.ref] as they are, without P3574's
# constexpr. specgen checks the revision out into a scratch tree for the run,
# so the tag has to be in the clone.
#
# WORDING_DIFF_CONTEXT is specgen's --diff-context: changed (whole changed
# subclauses), items (unchanged runs of items elided as [...]), or all.
#
# A baseline revision is not a file make can date, so the target always runs.

WORDING_BASELINE ?= p3574-baseline-draft
WORDING_DIFF_CONTEXT ?= changed
WORDING_DIFF_DIR := $(WORDING_DIR)/diff
WORDING_DIFF_STAGE := $(BUILD_DIR)/wording-diff-stage

.PHONY: wording-diff
wording-diff: ## Regenerate papers/P3574/wording/diff/, the wording marked against the working draft
	@rm -rf $(WORDING_DIFF_STAGE)
	@mkdir -p $(WORDING_DIFF_STAGE) $(WORDING_DIFF_DIR)/fragments
	@$(call specgen_wording,$(WORDING_DIFF_STAGE), \
	    --baseline-rev $(WORDING_BASELINE) --diff-context $(WORDING_DIFF_CONTEXT))
	@for clause in $(WORDING_CLAUSES); do \
	    cp $(WORDING_DIFF_STAGE)/$$clause.tex $(WORDING_DIFF_DIR)/fragments/$$clause.tex; \
	done
	@$(call assemble_wording,$(WORDING_DIFF_DIR),$(WORDING_DIR)/diff-preamble.tex)
	@echo "Wrote $(WORDING_DIFF_DIR)/wording.tex"
