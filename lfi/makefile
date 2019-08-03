

CFLAGS=-O2 -g

lfi.x: liblfi.a lfi.c
	cc -g -o lfi.x lfi.c -L. -llfi

liblfi.a: iswap8.o  lfi_abor.o  lfi_altm.o  lfi_alts.o  lfi_dumm.o  lfi_fmul.o  lfi_grok.o  lfi_hndl.o  lfi_intf.o  lfi_miss.o  lfi_util.o  lfi_verb.o
	ar crv liblfi.a *.o


%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<


clean:
	\rm -f *.a *.x *.o

