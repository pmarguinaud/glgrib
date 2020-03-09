

CFLAGS=-fPIC -O2 -g

all: liblfi.so

liblfi.so: iswap8.o  lfi_abor.o  lfi_altm.o  lfi_alts.o  lfi_dumm.o  lfi_fmul.o  lfi_grok.o  lfi_hndl.o  lfi_intf.o  lfi_miss.o  lfi_util.o  lfi_verb.o lfi_netw.o
	gcc -shared -o liblfi.so *.o


%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<


clean:
	\rm -f *.so *.x *.o

