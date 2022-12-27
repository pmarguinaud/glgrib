TOP=../..

include $(TOP)/makefile.inc


all: $(TOP)/lib/libLFI.so.0 

$(TOP)/lib/libLFI.so.0: iswap8.o  abor.o  altm.o  alts.o  dumm.o  fmul.o  grok.o  hndl.o  intf.o  miss.o  util.o  verb.o netw.o
	gcc -shared -Wl,-soname,libLFI.so.0 -o $(TOP)/lib/libLFI.so.0 *.o $(LIBS_LFI)
	@\ln -sf libLFI.so.0 $(TOP)/lib/libLFI.so

%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<

clean:
	\rm -f *.o $(TOP)/lib/libLFI.so* 
