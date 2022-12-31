
all: blib/lib/Tk/glGrib.pm

install: Makefile
	$(MAKE) install -f Makefile

Makefile:
	perl Makefile.PL

blib/lib/Tk/glGrib.pm: lib/Tk/glGrib.pm Makefile
	$(MAKE) -f Makefile

clean: Makefile
	$(MAKE) clean -f Makefile
	\rm -f Makefile.old

test: Makefile
	$(MAKE) test -f Makefile
