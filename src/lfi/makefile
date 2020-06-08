TOP=../..

CFLAGS=-fPIC -O2 

all: $(TOP)/lib/libLFI.so $(TOP)/lib/libLFI.a

$(TOP)/lib/libLFI.so: iswap8.o  lfi_abor.o  lfi_altm.o  lfi_alts.o  lfi_dumm.o  lfi_fmul.o  lfi_grok.o  lfi_hndl.o  lfi_intf.o  lfi_miss.o  lfi_util.o  lfi_verb.o lfi_netw.o
	gcc -shared -o $(TOP)/lib/libLFI.so *.o

$(TOP)/lib/libLFI.a: iswap8.o  lfi_abor.o  lfi_altm.o  lfi_alts.o  lfi_dumm.o  lfi_fmul.o  lfi_grok.o  lfi_hndl.o  lfi_intf.o  lfi_miss.o  lfi_util.o  lfi_verb.o lfi_netw.o
	ar crv $(TOP)/lib/libLFI.a *.o


%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<

clean:
	\rm -f *.o $(TOP)/lib/libLFI.so $(TOP)/lib/libLFI.a
