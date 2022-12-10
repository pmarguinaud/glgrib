ifeq ($(BUILD), )
SUFFIX=
endif

ifeq ($(BUILD), batch)
SUFFIX=-batch
endif

LIBS_GLGRIB=-lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread \
    -lsqlite3 -lcurl -lshp -leccodes -lnetcdf_c++4 -lnetcdf
LIBS_LFI=-lsqlite3 -lcurl

LDFLAGS=-fopenmp -lglGrib$(SUFFIX) -L$(TOP)/lib -Wl,-rpath,$(ROOT)/lib -lLFI 

CXXFLAGS_BASE=-fPIC -fopenmp -std=c++11 -Wall 

CXXFLAGS=$(CXXFLAGS_BASE) -O0 -g -DGLGRIB_CHECK_BOUND
CXXFLAGS=$(CXXFLAGS_BASE) -O2 

ifeq ($(BUILD), )
LIBS_GLGRIB+=-lGLEW -lGL -lglfw
CXXFLAGS+=-DGLGRIB_USE_GLFW
endif

ifeq ($(BUILD), batch)
LIBS_GLGRIB+=-lEGL -lGL -lgbm
CXXFLAGS+=-DGLGRIB_USE_EGL
endif
