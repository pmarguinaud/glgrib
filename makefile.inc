
PREFIX=$(HOME)/3d/glgrib

LDFLAGS_GLFW=-lGLEW -lGL -lglfw
LDFLAGS_EGL=-lEGL -lGL -lgbm

NETCDF_PREFIX=/home/marguina/install/netcdf-c-4.7.4
ECCODES_PREFIX=/home/marguina/install/eccodes-2.14.0

LDFLAGS=-fopenmp -lglGrib -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib \
        -lLFI -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread -lsqlite3 -lcurl -lshp \
        -L$(ECCODES_PREFIX)/lib -Wl,-rpath,$(ECCODES_PREFIX)/lib -leccodes \
        -L$(NETCDF_PREFIX)/lib -Wl,-rpath,$(NETCDF_PREFIX)/lib -lnetcdf_c++4 -lnetcdf

CXXFLAGS=-fPIC -O0 -fopenmp -std=c++11 -Wall -g -I$(ECCODES_PREFIX)/include -I$(NETCDF_PREFIX)/include -DGLGRIB_CHECK_BOUNDS
CXXFLAGS=-fPIC -O2 -fopenmp -std=c++11 -Wall -I$(ECCODES_PREFIX)/include -I$(NETCDF_PREFIX)/include

CXXFLAGS+=-DUSE_GLFW
LDFLAGS+=$(LDFLAGS_GLFW)

