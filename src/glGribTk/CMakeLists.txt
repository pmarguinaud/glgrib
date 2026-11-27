# glgrib-tk: pure Perl (Perl/Tk interface), nothing to compile

install (PROGRAMS bin/glgrib-tk DESTINATION ${CMAKE_INSTALL_BINDIR})

file (GLOB tk_modules CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/Tk/glGrib*.pm)
install (FILES ${tk_modules} DESTINATION ${GLGRIB_PERL_LIB_DIR}/Tk)

# Man page glgrib-tk.1p
set (man1p ${CMAKE_BINARY_DIR}/share/man/man1/glgrib-tk.1p)
add_custom_command (
  OUTPUT  ${man1p}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/share/man/man1
  COMMAND ${POD2MAN} --section=1p ${CMAKE_CURRENT_SOURCE_DIR}/bin/glgrib-tk ${man1p}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bin/glgrib-tk
  COMMENT "Generating man page glgrib-tk.1p"
  VERBATIM
)
add_custom_target (man-glgrib-tk ALL DEPENDS ${man1p})
install (FILES ${man1p} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
