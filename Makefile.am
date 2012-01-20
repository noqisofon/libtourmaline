SUBDIRS = src

if ENABLE_TESTS
SUBDIRS += tests
endif

DIST_SUBDIRS = src tests

dist-hook: changelog

changelog: $(top_srcdir)/.git/HEAD
	git log --stat > ChangeLog

doc: doxygen.conf
	doxygen doxygen.conf
