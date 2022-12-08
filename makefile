
all:
	@mkdir -p bin lib
	cd src/lfi && make -j4 && cd .. && make -j4

clean:
	\rm -rf share/glgrib/shaders/
	cd src && make clean
	cd src/lfi && make clean 


