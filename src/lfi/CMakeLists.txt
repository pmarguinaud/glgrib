
set (SRC
  iswap8.c
  lfi_abor.c
  lfi_altm.c
  lfi_alts.c
  lfi_dumm.c
  lfi_fmul.c
  lfi_grok.c
  lfi_hndl.c
  lfi_intf.c
  lfi_miss.c
  lfi_netw.c
  lfi_util.c
  lfi_verb.c
)

add_library (LFI SHARED ${SRC})
install (TARGETS LFI DESTINATION lib)

