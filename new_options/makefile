

CXXFLAGS=-fopenmp -std=c++11 -g -I$(HOME)/3d/usr/include -L$(HOME)/3d/usr/lib64 -Wl,-rpath,$(HOME)/3d/usr/lib64 -leccodes -lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto

all: main.x 
	
main.x: main.cc 
	g++ $(CXXFLAGS) -o main.x main.cc
