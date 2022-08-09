
ifeq ($(BUILD), )
SUFFIX=
endif

ifeq ($(BUILD), batch)
SUFFIX=_batch
endif

PREFIX=$(HOME)/3d/glgrib


LDFLAGS=-fopenmp -lglGrib$(SUFFIX) -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib \
        -lLFI -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread \
        -lsqlite3 -lcurl -lshp -leccodes -lnetcdf_c++4 -lnetcdf

CXXFLAGS_BASE=-fPIC -fopenmp -std=c++11 -Wall 

CXXFLAGS=$(CXXFLAGS_BASE) -O0 -g -DGLGRIB_CHECK_BOUNDS
CXXFLAGS=$(CXXFLAGS_BASE) -O2 

ifeq ($(BUILD), )
LDFLAGS_GLFW=-lGLEW -lGL -lglfw
CXXFLAGS+=-DGLGRIB_USE_GLFW
LDFLAGS+=$(LDFLAGS_GLFW)
endif

ifeq ($(BUILD), batch)
LDFLAGS_EGL=-lEGL -lGL -lgbm
CXXFLAGS+=-DGLGRIB_USE_EGL
LDFLAGS+=$(LDFLAGS_EGL)
endif



