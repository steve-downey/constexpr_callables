# The venv the paper builds in. minted runs TeX Live's latexminted under the
# venv's python3, and the latexminted that goes with TeX Live's minted.sty 3.7
# fails on Python 3.14's argparse, so the venv is pinned to 3.13; uv fetches
# it if this machine has no 3.13. Override with PAPER_PYTHON=<version>.
UV ?= uv
PAPER_PYTHON ?= 3.13
VENV := .venv/
SOURCE_VENV := . $(VENV)/bin/activate;
DEPS_DIR := .deps

override DEPS := $(VENV) wg21.bib

default: papers

$(VENV):
	$(UV) venv --python $(PAPER_PYTHON) --system-site-packages $(VENV)
	$(UV) pip sync --python $(VENV)/bin/python requirements.txt

.PHONY: create-venv
create-venv:
	rm -rf $(VENV)
	make $(VENV)

$(DEPS_DIR):
	mkdir -p $(DEPS_DIR)

.PHONY: papers
papers: constexpr-function-wording.pdf

constexpr-function-wording.pdf : constexpr-function-wording.tex wg21.bib $(DEPS_DIR) | $(VENV)
	$(SOURCE_VENV) latexmk -f -shell-escape -pdflua -use-make -deps -deps-out=$(DEPS_DIR)/$@.d -MP constexpr-function-wording.tex


define curl_cmd =
	curl https://wg21.link/index.bib > wg21.bib
endef

wg21.bib:
	$(curl_cmd)

.PHONY: curl-bib
curl-bib:  ## Download wg21.bib from wg21.link
	$(curl_cmd)

.PHONY: clean
clean:
	latexmk -c
	@$(MAKE) -f wg21/Makefile clean
	-rm *.pdf

# Include dependencies
# $(foreach file,$(TARGET),$(eval -include $(DEPS_DIR)/$(file).d))
-include $(DEPS_DIR)/*.d

help: ## Show this help.
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'  $(MAKEFILE_LIST)  | sort
