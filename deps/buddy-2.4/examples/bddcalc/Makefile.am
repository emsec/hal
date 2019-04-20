include ../Makefile.def
EXTRA_DIST = \
 readme \
 example.cal \
 examples/c432.cal \
 examples/c499.cal \
 examples/c1355.cal \
 examples/c1908.cal \
 examples/c2670.cal \
 examples/c3540.cal \
 examples/readme \
 runtest \
 expected

AM_YFLAGS = -d

BUILT_SOURCES = parser.h

check_PROGRAMS = bddcalc
bddcalc_SOURCES = \
 hashtbl.h \
 hashtbl.cxx \
 lexer.lxx \
 parser.yxx \
 parser_.h \
 slist.h
