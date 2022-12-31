
export ROOT=$(shell pwd)

all:
	@mkdir -p bin lib
	cd src && $(MAKE)
	@mkdir -p share/man/man1
	LD_LIBRARY_PATH=lib ./bin/glgrib      --pod | pod2man -r "" -n glgrib      -c "" | gzip -c > share/man/man1/glgrib.1.gz
	LD_LIBRARY_PATH=lib ./bin/glgrib-egl  --pod | pod2man -r "" -n glgrib-egl  -c "" | gzip -c > share/man/man1/glgrib-egl.1.gz
	LD_LIBRARY_PATH=lib ./bin/glgrib-glfw --pod | pod2man -r "" -n glgrib-glfw -c "" | gzip -c > share/man/man1/glgrib-glfw.1.gz

clean:
	cd src && make clean
	./scripts/debian/clean.sh
	@\rm -f share/man/man1/glgrib.1.gz share/man/man1/glgrib-glfw.1.gz share/man/man1/glgrib-egl.1.gz

