
SUFFIX=
PREFIX=$(HOME)/3d/glgrib

LDFLAGS_GLFW=-lGLEW -lGL -lglfw
LDFLAGS_EGL=-lEGL -lGL -lgbm

LDFLAGS=-fopenmp -lglGrib$(SUFFIX) -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib \
        -lLFI$(SUFFIX) -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread \
        -lsqlite3 -lcurl -lshp -leccodes -lnetcdf_c++4 -lnetcdf

CXXFLAGS_BASE=-fPIC -fopenmp -std=c++11 -Wall 

CXXFLAGS=$(CXXFLAGS_BASE) -O0 -g -DGLGRIB_CHECK_BOUNDS
CXXFLAGS=$(CXXFLAGS_BASE) -O2 

CXXFLAGS+=-DGLGRIB_USE_GLFW
LDFLAGS+=$(LDFLAGS_GLFW)


