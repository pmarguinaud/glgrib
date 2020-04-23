
ECCODES_PREFIX=$(HOME)/install/eccodes-2.14.0

PREFIX=$(HOME)/3d/glgrib
PREFIX2=$(HOME)/3d/usr

LDFLAGS=-fopenmp \
        -Wl,-rpath,$(PREFIX)/lib -L$(PREFIX)/lib -lglGrib -lLFI \
        -L$(ECCODES_PREFIX)/lib -Wl,-rpath,$(ECCODES_PREFIX)/lib -leccodes \
	-Wl,-rpath,$(PREFIX2)/lib64 -L$(PREFIX2)/lib64 -lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread -lsqlite3 -lcurl -lshp 

CXXFLAGS=-fPIC -O2 -fopenmp -std=c++11 -g -I$(ECCODES_PREFIX)/include  -I $(PREFIX2)/include  -Wall

