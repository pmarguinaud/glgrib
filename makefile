
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
	\rm -rf share/glgrib/shaders/
	\rm -rf debian/.debhelper/
	\rm -rf debian/debhelper-build-stamp
	\rm -rf debian/files
	\rm -rf debian/glgrib.substvars
	\rm -rf debian/glgrib/
	cd src && make clean
	cd src/lfi && make clean 

