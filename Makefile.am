SUBDIRS         = src test
EXTRA_DIST      = AUTHORS UNLICENSE VERSION
ACLOCAL_AMFLAGS = -I .aclocal

README.html: README.rst
	$(PANDOC) -o $@ -t html5 -s $<

CLEANFILES = README.html
