
ECCODES_PREFIX=$(HOME)/install/eccodes-2.14.0

PREFIX=$(HOME)/3d/glgrib

LDFLAGS=-fopenmp \
        -Wl,-rpath,$(PREFIX)/lib -L$(PREFIX)/lib -lglGrib -lLFI \
        -L$(ECCODES_PREFIX)/lib -Wl,-rpath,$(ECCODES_PREFIX)/lib -leccodes \
        -L$(HOME)/3d/usr/lib64 -Wl,-rpath,$(HOME)/3d/usr/lib64 -L$(HOME)/3d/usr/lib -Wl,-rpath,$(HOME)/3d/usr/lib \
        -lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread -lsqlite3 -lcurl -lshp 

CXXFLAGS=-fPIC -O2 -fopenmp -std=c++11 -g -I$(HOME)/3d/usr/include -I$(ECCODES_PREFIX)/include  

