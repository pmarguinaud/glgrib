
all: blib/lib/Tk/glGrib.pm

install: Makefile
	make install -f Makefile

Makefile:
	perl Makefile.PL

blib/lib/Tk/glGrib.pm: lib/Tk/glGrib.pm Makefile
	make -f Makefile

clean: Makefile
	make clean -f Makefile
	\rm -f Makefile.old

test: Makefile
	make test -f Makefile
