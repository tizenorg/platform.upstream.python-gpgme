PYTHON = python2.4

build:
	$(PYTHON) setup.py build_ext -i

check: build
	$(PYTHON) test_all.py -v

clean:
	$(PYTHON) setup.py clean

dist: build
	$(PYTHON) setup.py sdist --force-manifest

.PHONY: build check clean dist
