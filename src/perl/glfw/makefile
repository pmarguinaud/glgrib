
all: blib/arch/auto/glGrib/glGrib/glfw.so

install: Makefile
	make install -f Makefile

Makefile:
	perl Makefile.PL

blib/arch/auto/glGrib/glGrib/glfw.so: glfw.xs Makefile
	make -f Makefile

clean: Makefile
	make clean -f Makefile

test: Makefile
	make test -f Makefile
