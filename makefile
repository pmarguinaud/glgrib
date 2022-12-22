
export ROOT=$(shell pwd)

all:
	@mkdir -p bin lib
	cd src && make 

clean:
	cd src && make clean
	./scripts/debian/clean.sh

