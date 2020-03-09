
all: blib/arch/auto/glGrib/glGrib.so

blib/arch/auto/glGrib/glGrib.so: glGrib.xs
	perl Makefile.PL && make -f Makefile

clean:
	make clean -f Makefile

test:
	make test -f Makefile
