
TOP=../..

include $(TOP)/makefile.inc

glGrib/glfw.so: glfw.cc
	@mkdir -p glGrib
	g++ -DGLGRIB_PREFIX=\"`pwd`/$(TOP)\" $(CXXFLAGS) -I$(TOP)/include -DUSE_GLFW  --shared `python3-config --includes` -O2 glfw.cc -o glGrib/glfw.so $(LDFLAGS) -Wl,-rpath,$(TOP)/lib

clean:
	\rm -rf glGrib

test:
	./glGribWindow.py
	./glGribOffline.py
