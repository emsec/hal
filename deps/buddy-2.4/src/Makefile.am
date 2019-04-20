AM_CPPFLAGS = -I$(top_builddir)

include_HEADERS = bdd.h fdd.h bvec.h

lib_LTLIBRARIES = libbdd.la
libbdd_la_SOURCES = \
 bddio.c \
 bddop.c \
 bddtree.h \
 bvec.c \
 cache.c \
 cache.h \
 cppext.cxx \
 fdd.c \
 imatrix.c \
 imatrix.h \
 kernel.c \
 kernel.h \
 pairs.c \
 prime.c \
 prime.h \
 reorder.c \
 tree.c

# See the `Updating version info' node of the Libtool manual before
# changing this.
libbdd_la_LDFLAGS = -version-info 0:0:0

check_PROGRAMS = bddtest
bddtest_SOURCES = bddtest.cxx
bddtest_LDADD = ./libbdd.la
