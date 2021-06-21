
PREFIX=$(HOME)/3d/glgrib

LDFLAGS_GLFW=-lGLEW -lGL -lglfw
LDFLAGS_EGL=-lEGL -lGL -lgbm

NETCDF_PREFIX=/home/marguina/install/netcdf-c-4.7.4
ECCODES_PREFIX=/home/marguina/install/eccodes-2.14.0
SQLITE3_PREFIX=/home/marguina/install/sqlite-3.34.0

LDFLAGS=-fopenmp -lglGrib -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib \
        -lLFI -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread \
        -L$(SQLITE3_PREFIX)/lib -Wl,-rpath,$(SQLITE3_PREFIX)/lib -lsqlite3 -lcurl -lshp \
        -L$(ECCODES_PREFIX)/lib -Wl,-rpath,$(ECCODES_PREFIX)/lib -leccodes \
        -L$(NETCDF_PREFIX)/lib -Wl,-rpath,$(NETCDF_PREFIX)/lib -lnetcdf_c++4 -lnetcdf

CXXFLAGS_BASE=-fPIC -fopenmp -std=c++11 -Wall -I$(SQLITE3_PREFIX)/include -I$(ECCODES_PREFIX)/include -I$(NETCDF_PREFIX)/include

CXXFLAGS=$(CXXFLAGS_BASE) -O0 -g -DGLGRIB_CHECK_BOUNDS
CXXFLAGS=$(CXXFLAGS_BASE) -O2 

#CXXFLAGS+=-DGLGRIB_USE_EGL -DGLGRIB_USE_GBM
#LDFLAGS+=$(LDFLAGS_EGL)
CXXFLAGS+=-DGLGRIB_USE_GLFW
LDFLAGS+=$(LDFLAGS_GLFW)


