
PREFIX=$(HOME)/3d/glgrib

LDFLAGS=-fopenmp \
        -Wl,-rpath,$(PREFIX)/lib -L$(PREFIX)/lib -lglGrib -lLFI \
        -L/home/marguina/install/eccodes-2.14.0/lib -Wl,-rpath,/home/marguina/install/eccodes-2.14.0/lib -leccodes \
	-L/home/marguina/install/netcdf-c-4.7.4/lib -Wl,-rpath,/home/marguina/install/netcdf-c-4.7.4/lib \
	-lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread -lsqlite3 -lcurl -lshp -leccodes -lnetcdf_c++4 -lnetcdf

#CXXFLAGS=-fPIC -O0 -fopenmp -std=c++11 -Wall -g
CXXFLAGS=-fPIC -O2 -fopenmp -std=c++11 -Wall -I/home/marguina/install/eccodes-2.14.0/include -I/home/marguina/install/netcdf-c-4.7.4/include

