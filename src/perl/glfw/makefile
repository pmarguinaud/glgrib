
all: blib/arch/auto/glGrib/glGrib/glfw.so

blib/arch/auto/glGrib/glGrib/glfw.so: glfw.xs
	perl Makefile.PL && make -f Makefile

clean:
	make clean -f Makefile

test:
	make test -f Makefile
