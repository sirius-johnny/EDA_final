bin_PROGRAMS = ntuplace


# set the include path found by configure
INCLUDES = -I$(top_srcdir)/include $(all_includes)

# the library search path.
ntuplace_LDFLAGS = -all-static $(all_libraries) -L$(top_srcdir)/lib -lhmetis \
	-lclef -llef -ldef -lm -lsolverQP -llpsolve55 -ldl -lmetis 
ntuplace_SOURCES = BinPacking.cpp defrw.cpp detail.cpp DiamondLegalizer.cpp \
			DiamondSearch.cpp FMPart.cpp lefdef.cpp lefrw.cpp legal.cpp main.cpp ParamPlacement.cpp \
			ParserIBMMix.cpp Partition.cpp Prelegalizer.cpp mincutplacer.cpp placedb.cpp \
			randomplace.cpp placedb_bookshelf.cpp metalegal.cpp bbsolve.cpp bbcellswap.cpp util.cpp \
			MyNLP.cpp mincutcluster.cpp PlaceDBQP.cpp fccluster.cpp macrolegal.cpp \
			cluster.cpp mlnlp.cpp DPlace.cpp TetrisLegal.cpp placebin.cpp \
	cellmoving.cpp smooth.cpp boost_maxflow.cpp
noinst_HEADERS = BinPacking.h detail.h DiamondLegalizer.h DiamondSearch.h \
	FMPart.h lefdef.h legal.h ParserBookshelf.h ParserIBMMix.h patoh.h Placement.h \
	Prelegalizer.h util.h ParamPlacement.h Partition.h mincutplacer.h placedb.h \
	randomplace.h placeutil.h bbsolve.h bbcellswap.h MyNLP.h mincutcluster.h PlaceDBQP.h \
	solverQP.h fccluster.h macrolegal.h cluster.h mlnlp.h DPlace.h \
	TetrisLegal.h placebin.h cellmoving.h smooth.h


METASOURCES = AUTO
BUILD_HOST = "`hostname`"
BUILD_DATE = "`export LANG=C;date`"
BUILD_ARCH = "`uname -mo`"
CXX_VERSION = "`$(CXX) --version|head -n 1`"
NTUPLACE_VERSION = "`date +V6.%m.%d`"
AM_CXXFLAGS = -g -Wall -m32 -fomit-frame-pointer -funroll-loops \
	-D__BUILD_DATE__=\"$(BUILD_DATE)\" 		-D__BUILD_USER__=\"$(USER)\" -D__BUILD_HOST__=\"$(BUILD_HOST)\" \
	-D__BUILD_ARCH__=\"$(BUILD_ARCH)\" 		-D__CXXFLAGS__=\"'$(CXXFLAGS)'\" \
	-D__LDFLAGS__=\"'$(ntuplace_LDFLAGS)'\" 		-D__NTUPLACE_VERSION__=\"$(NTUPLACE_VERSION)\" \
	-D__CXX_VERSION__=\"$(CXX_VERSION)\"

