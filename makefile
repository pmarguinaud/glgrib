
all:
	cd src/lfi && make -j4 && cd .. && make -j4

clean:
	cd src/lfi && make clean && cd .. && make clean


