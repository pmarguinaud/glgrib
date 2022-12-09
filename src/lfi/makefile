TOP=../..

include $(TOP)/makefile.inc

CFLAGS=-fPIC -O2 

all: $(TOP)/lib/libLFI.so.0 

$(TOP)/lib/libLFI.so.0: iswap8.o  lfi_abor.o  lfi_altm.o  lfi_alts.o  lfi_dumm.o  lfi_fmul.o  lfi_grok.o  lfi_hndl.o  lfi_intf.o  lfi_miss.o  lfi_util.o  lfi_verb.o lfi_netw.o
	gcc -shared -Wl,-soname,libLFI.so.0 -o $(TOP)/lib/libLFI.so.0 *.o $(LIBS_LFI)
	@\ln -sf libLFI.so.0 $(TOP)/lib/libLFI.so

%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<

clean:
	\rm -f *.o $(TOP)/lib/libLFI.so* 
