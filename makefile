
export ROOT=$(shell pwd)

ifneq ($(DEST), )
install: all
	./scripts/install.pl
endif

all:
	@mkdir -p bin lib
	cd src/lfi && make -j4 
	cd src && make -j4

clean:
	cd src && make clean
	cd src/lfi && make clean 

