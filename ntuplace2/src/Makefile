# Makefile.in generated automatically by automake 1.4-p6 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/bash

srcdir = .
top_srcdir = ..

prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/share/info
mandir = ${prefix}/share/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/ntuplace
pkglibdir = $(libdir)/ntuplace
pkgincludedir = $(includedir)/ntuplace

top_builddir = ..

ACLOCAL = aclocal-1.4
AUTOCONF = autoconf
AUTOMAKE = automake-1.4
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
host_alias = 
host_triplet = x86_64-unknown-linux-gnu
AR = ar
ARGZ_H = @ARGZ_H@
AS = @AS@
CC = gcc
CXX = g++-4.2
CXXCPP = g++-4.2 -E
DLLTOOL = @DLLTOOL@
DSYMUTIL = 
DUMPBIN = 
F77 = @F77@
GCJ = @GCJ@
GCJFLAGS = @GCJFLAGS@
GREP = /bin/grep
INCLTDL = @INCLTDL@
LD = /usr/bin/ld -m elf_x86_64
LIBADD_DL = @LIBADD_DL@
LIBADD_DLD_LINK = @LIBADD_DLD_LINK@
LIBADD_DLOPEN = @LIBADD_DLOPEN@
LIBADD_SHL_LOAD = @LIBADD_SHL_LOAD@
LIBLTDL = @LIBLTDL@
LIBM = @LIBM@
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIPO = 
LN_S = ln -s
LTDLDEPS = @LTDLDEPS@
LTDLINCL = @LTDLINCL@
LTDLOPEN = @LTDLOPEN@
LT_CONFIG_H = @LT_CONFIG_H@
LT_DLLOADERS = @LT_DLLOADERS@
LT_DLPREOPEN = @LT_DLPREOPEN@
MAKEINFO = /home/kaie/research/NTUplace/test/ntuplace2.0/missing makeinfo
NM = /usr/bin/nm -B
NMEDIT = 
OBJDUMP = objdump
OTOOL = 
OTOOL64 = 
PACKAGE = ntuplace
RANLIB = ranlib
RC = @RC@
SED = /bin/sed
STRIP = strip
VERSION = 0.3
lt_ECHO = echo
ltdl_LIBOBJS = @ltdl_LIBOBJS@
ltdl_LTLIBOBJS = @ltdl_LTLIBOBJS@
sys_symbol_underscore = @sys_symbol_underscore@

bin_PROGRAMS = ntuplace

# set the include path found by configure
INCLUDES = -I$(top_srcdir)/include $(all_includes)

# the library search path.
ntuplace_LDFLAGS = -all-static $(all_libraries) -L$(top_srcdir)/lib -lhmetis 	-lclef -llef -ldef -lm -lsolverQP -llpsolve55 -ldl -lmetis 

ntuplace_SOURCES = BinPacking.cpp defrw.cpp detail.cpp DiamondLegalizer.cpp 			DiamondSearch.cpp FMPart.cpp lefdef.cpp lefrw.cpp legal.cpp main.cpp ParamPlacement.cpp 			ParserIBMMix.cpp Partition.cpp Prelegalizer.cpp mincutplacer.cpp placedb.cpp 			randomplace.cpp placedb_bookshelf.cpp metalegal.cpp bbsolve.cpp bbcellswap.cpp util.cpp 			MyNLP.cpp mincutcluster.cpp PlaceDBQP.cpp fccluster.cpp macrolegal.cpp 			cluster.cpp mlnlp.cpp DPlace.cpp TetrisLegal.cpp placebin.cpp 	cellmoving.cpp smooth.cpp boost_maxflow.cpp

noinst_HEADERS = BinPacking.h detail.h DiamondLegalizer.h DiamondSearch.h 	FMPart.h lefdef.h legal.h ParserBookshelf.h ParserIBMMix.h patoh.h Placement.h 	Prelegalizer.h util.h ParamPlacement.h Partition.h mincutplacer.h placedb.h 	randomplace.h placeutil.h bbsolve.h bbcellswap.h MyNLP.h mincutcluster.h PlaceDBQP.h 	solverQP.h fccluster.h macrolegal.h cluster.h mlnlp.h DPlace.h 	TetrisLegal.h placebin.h cellmoving.h smooth.h


METASOURCES = AUTO
BUILD_HOST = "`hostname`"
BUILD_DATE = "`export LANG=C;date`"
BUILD_ARCH = "`uname -mo`"
CXX_VERSION = "`$(CXX) --version|head -n 1`"
NTUPLACE_VERSION = "`date +V6.%m.%d`"
AM_CXXFLAGS = -g -Wall -m32 -fomit-frame-pointer -funroll-loops 	-D__BUILD_DATE__=\"$(BUILD_DATE)\" 		-D__BUILD_USER__=\"$(USER)\" -D__BUILD_HOST__=\"$(BUILD_HOST)\" 	-D__BUILD_ARCH__=\"$(BUILD_ARCH)\" 		-D__CXXFLAGS__=\"'$(CXXFLAGS)'\" 	-D__LDFLAGS__=\"'$(ntuplace_LDFLAGS)'\" 		-D__NTUPLACE_VERSION__=\"$(NTUPLACE_VERSION)\" 	-D__CXX_VERSION__=\"$(CXX_VERSION)\"

mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES = 
PROGRAMS =  $(bin_PROGRAMS)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I..
CPPFLAGS = 
LDFLAGS = 
LIBS = 
ntuplace_OBJECTS =  BinPacking.o defrw.o detail.o DiamondLegalizer.o \
DiamondSearch.o FMPart.o lefdef.o lefrw.o legal.o main.o \
ParamPlacement.o ParserIBMMix.o Partition.o Prelegalizer.o \
mincutplacer.o placedb.o randomplace.o placedb_bookshelf.o metalegal.o \
bbsolve.o bbcellswap.o util.o MyNLP.o mincutcluster.o PlaceDBQP.o \
fccluster.o macrolegal.o cluster.o mlnlp.o DPlace.o TetrisLegal.o \
placebin.o cellmoving.o smooth.o boost_maxflow.o
ntuplace_LDADD = $(LDADD)
ntuplace_DEPENDENCIES = 
CXXFLAGS = -g -O2
CXXCOMPILE = $(CXX) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
LTCXXCOMPILE = $(LIBTOOL) --mode=compile $(CXX) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
CXXLD = $(CXX)
CXXLINK = $(LIBTOOL) --mode=link $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@
HEADERS =  $(noinst_HEADERS)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
DEP_FILES =  .deps/BinPacking.P .deps/DPlace.P .deps/DiamondLegalizer.P \
.deps/DiamondSearch.P .deps/FMPart.P .deps/MyNLP.P \
.deps/ParamPlacement.P .deps/ParserIBMMix.P .deps/Partition.P \
.deps/PlaceDBQP.P .deps/Prelegalizer.P .deps/TetrisLegal.P \
.deps/bbcellswap.P .deps/bbsolve.P .deps/boost_maxflow.P \
.deps/cellmoving.P .deps/cluster.P .deps/defrw.P .deps/detail.P \
.deps/fccluster.P .deps/lefdef.P .deps/lefrw.P .deps/legal.P \
.deps/macrolegal.P .deps/main.P .deps/metalegal.P .deps/mincutcluster.P \
.deps/mincutplacer.P .deps/mlnlp.P .deps/placebin.P .deps/placedb.P \
.deps/placedb_bookshelf.P .deps/randomplace.P .deps/smooth.P \
.deps/util.P
SOURCES = $(ntuplace_SOURCES)
OBJECTS = $(ntuplace_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .cpp .lo .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu src/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	    $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	done

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

.s.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

.S.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

mostlyclean-libtool:
	-rm -f *.lo

clean-libtool:
	-rm -rf .libs _libs

distclean-libtool:

maintainer-clean-libtool:

ntuplace: $(ntuplace_OBJECTS) $(ntuplace_DEPENDENCIES)
	@rm -f ntuplace
	$(CXXLINK) $(ntuplace_LDFLAGS) $(ntuplace_OBJECTS) $(ntuplace_LDADD) $(LIBS)
.cpp.o:
	$(CXXCOMPILE) -c $<
.cpp.lo:
	$(LTCXXCOMPILE) -c $<

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS:  $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags -o $$here/TAGS $(ETAGS_ARGS) $$tags  $$unique $(LISP))

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = src

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu src/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

-include $(DEP_FILES)

mostlyclean-depend:

clean-depend:

distclean-depend:
	-rm -rf .deps

maintainer-clean-depend:

%.o: %.c
	@echo '$(COMPILE) -c $<'; \
	$(COMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

%.lo: %.c
	@echo '$(LTCOMPILE) -c $<'; \
	$(LTCOMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-sed -e 's/^\([^:]*\)\.o[ 	]*:/\1.lo \1.o :/' \
	  < .deps/$(*F).pp > .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm -f .deps/$(*F).pp

%.o: %.cpp
	@echo '$(CXXCOMPILE) -c $<'; \
	$(CXXCOMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

%.lo: %.cpp
	@echo '$(LTCXXCOMPILE) -c $<'; \
	$(LTCXXCOMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-sed -e 's/^\([^:]*\)\.o[ 	]*:/\1.lo \1.o :/' \
	  < .deps/$(*F).pp > .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm -f .deps/$(*F).pp
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am: install-binPROGRAMS
install-exec: install-exec-am

install-data-am:
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-binPROGRAMS
uninstall: uninstall-am
all-am: Makefile $(PROGRAMS) $(HEADERS)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(bindir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-binPROGRAMS mostlyclean-compile \
		mostlyclean-libtool mostlyclean-tags mostlyclean-depend \
		mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-binPROGRAMS clean-compile clean-libtool clean-tags \
		clean-depend clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-binPROGRAMS distclean-compile distclean-libtool \
		distclean-tags distclean-depend distclean-generic \
		clean-am
	-rm -f libtool

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-binPROGRAMS \
		maintainer-clean-compile maintainer-clean-libtool \
		maintainer-clean-tags maintainer-clean-depend \
		maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: mostlyclean-binPROGRAMS distclean-binPROGRAMS clean-binPROGRAMS \
maintainer-clean-binPROGRAMS uninstall-binPROGRAMS install-binPROGRAMS \
mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile mostlyclean-libtool distclean-libtool \
clean-libtool maintainer-clean-libtool tags mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir \
mostlyclean-depend distclean-depend clean-depend \
maintainer-clean-depend info-am info dvi-am dvi check check-am \
installcheck-am installcheck install-exec-am install-exec \
install-data-am install-data install-am install uninstall-am uninstall \
all-redirect all-am all installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
