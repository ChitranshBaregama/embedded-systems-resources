PYTHON ?= python3
Q ?= Q0001
.PHONY: help check practice summary
help:
	@echo "make check | make practice Q=Q0001 | make summary"
check:
	$(PYTHON) scripts/learning.py validate
	$(PYTHON) -m unittest discover -s scripts -p 'test_learning.py'
	$(PYTHON) scripts/check-links.py
practice:
	$(PYTHON) scripts/learning.py run $(Q)
summary:
	$(PYTHON) scripts/learning.py summary
