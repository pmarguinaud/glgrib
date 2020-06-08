
PREFIX=$(HOME)/3d/glgrib

LDFLAGS=-fopenmp \
        -Wl,-rpath,$(PREFIX)/lib -L$(PREFIX)/lib -lglGrib -lLFI \
	-lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread -lsqlite3 -lcurl -lshp -leccodes -lnetcdf_c++4

#CXXFLAGS=-fPIC -O0 -fopenmp -std=c++11 -Wall -g
CXXFLAGS=-fPIC -O2 -fopenmp -std=c++11 -Wall

