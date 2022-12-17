
set (SRC
  iswap8.c
  abor.c
  altm.c
  alts.c
  dumm.c
  fmul.c
  grok.c
  hndl.c
  intf.c
  miss.c
  netw.c
  util.c
  verb.c
)

add_library (LFI SHARED ${SRC})

include_directories ("${CMAKE_SOURCE_DIR}/include")

install (TARGETS LFI DESTINATION lib)

