
ECCODES_PREFIX=$(HOME)/install/eccodes-2.14.0

LDFLAGS=-fopenmp \
        -L$(HOME)/3d/glgrib -Wl,-rpath,$(HOME)/3d/glgrib -lglGrib \
        -L$(HOME)/3d/glgrib/lfi -Wl,-rpath,$(HOME)/3d/glgrib/lfi -llfi -lcurl -lshp \
        -L$(ECCODES_PREFIX)/lib -Wl,-rpath,$(ECCODES_PREFIX)/lib -leccodes \
        -L$(HOME)/3d/usr/lib64 -Wl,-rpath,$(HOME)/3d/usr/lib64 -L$(HOME)/3d/usr/lib -Wl,-rpath,$(HOME)/3d/usr/lib \
        -lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread -lsqlite3 

CXXFLAGS=-fPIC -O2 -fopenmp -std=c++11 -g -I$(HOME)/3d/usr/include -I$(ECCODES_PREFIX)/include  

