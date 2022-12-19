
TOP=../..

include $(TOP)/makefile.inc

glGrib.so: glGrib.cc
	g++ -DGLGRIB_PREFIX=\"`pwd`/$(TOP)\" $(CXXFLAGS) -I$(TOP)/include -DUSE_GLFW  --shared `python2-config --includes` -O2 glGrib.cc -o glGrib.so $(LDFLAGS) -Wl,-rpath,$(TOP)/lib

clean:
	\rm -f glGrib.so

test:
	./glGribWindow.py
	./glGribOffline.py
