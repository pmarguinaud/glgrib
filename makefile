
export ROOT=$(shell pwd)

all:
	@mkdir -p bin lib
	cd src/lfi && make -j4 
	cd src && make -j4

clean:
	cd src && make clean
	cd src/lfi && make clean 

