
export ROOT=$(shell pwd)

all:
	@mkdir -p bin lib
	cd src && make 
	@mkdir -p share/man/man1
	LD_LIBRARY_PATH=lib ./bin/glgrib --pod | pod2man | gzip -c > share/man/man1/glgrib-gflw.1.gz
	LD_LIBRARY_PATH=lib ./bin/glgrib --pod | pod2man | gzip -c > share/man/man1/glgrib-egl.1.gz

clean:
	cd src && make clean
	./scripts/debian/clean.sh

