
include ../makefile.inc

glGrib.so: glGrib.cc
	g++ -DGLGRIB_PREFIX=\"$(HOME)/3d/glgrib\" $(CXXFLAGS) -I .. --shared `python-config --includes` -O0 glGrib.cc -o glGrib.so $(LDFLAGS)

clean:
	\rm -f glGrib.so
