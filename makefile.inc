
PREFIX=$(HOME)/3d/glgrib
PREFIX2=$(HOME)/3d/usr

LDFLAGS=-fopenmp \
        -Wl,-rpath,$(PREFIX)/lib -L$(PREFIX)/lib -lglGrib -lLFI \
	-Wl,-rpath,$(PREFIX2)/lib64 -L$(PREFIX2)/lib64 -lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread -lsqlite3 -lcurl -lshp -leccodes

CXXFLAGS=-fPIC -O0 -fopenmp -std=c++11 -g -I$(ECCODES_PREFIX)/include  -I $(PREFIX2)/include  -Wall

