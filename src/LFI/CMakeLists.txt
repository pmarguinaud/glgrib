add_library (LFI SHARED
  abor.c altm.c alts.c dumm.c fmul.c grok.c hndl.c intf.c iswap8.c miss.c
  netw.c util.c verb.c
)

# Same file names as the makefiles: libLFI.so.0 and libLFI.so
set_target_properties (LFI PROPERTIES VERSION 0 SOVERSION 0)

target_include_directories (LFI PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries (LFI PRIVATE ${GLGRIB_SQLITE3} CURL::libcurl)

install (TARGETS LFI
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          NAMELINK_COMPONENT dev
)

install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/LFI
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
