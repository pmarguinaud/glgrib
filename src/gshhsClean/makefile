# gshhs_clean : offline cleaning of coastline shapefiles (GSHHS, Natural Earth) for glgrib.
# Not part of the default build ; run "make" in this directory.
# Needs shapelib (-lshp) and CDT (vendored in include/CDT).

TOP=../..

include $(TOP)/makefile.inc

all: gshhs_clean

gshhs_clean: gshhs_clean.cc
	$(CXX) $(CXXFLAGS) -I$(TOP)/include -o $@ $< -fopenmp -lshp $(LDFLAGS_USER)

clean:
	\rm -f gshhs_clean
