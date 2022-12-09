ifeq ($(BUILD), )
SUFFIX=
endif

ifeq ($(BUILD), batch)
SUFFIX=-batch
endif

LIBS=-lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread \
    -lsqlite3 -lcurl -lshp -leccodes -lnetcdf_c++4 -lnetcdf

LDFLAGS=-fopenmp -lglGrib$(SUFFIX) -L$(TOP)/lib -Wl,-rpath,$(TOP)/lib -lLFI $(LIBS)

CXXFLAGS_BASE=-fPIC -fopenmp -std=c++11 -Wall 

CXXFLAGS=$(CXXFLAGS_BASE) -O0 -g -DGLGRIB_CHECK_BOUND
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
