

CFLAGS=-O2 -g

all: test.x

test.x: liblfi.a test.c
	cc -g -o test.x test.c -L. -llfi -lcurl -lsqlite3

liblfi.a: iswap8.o  lfi_abor.o  lfi_altm.o  lfi_alts.o  lfi_dumm.o  lfi_fmul.o  lfi_grok.o  lfi_hndl.o  lfi_intf.o  lfi_miss.o  lfi_util.o  lfi_verb.o lfi_netw.o
	ar crv liblfi.a *.o


%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<


clean:
	\rm -f *.a *.x *.o

