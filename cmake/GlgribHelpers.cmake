# Helpers for the glgrib build

# glgrib_add_backend (<name> SOURCES <sources...> LIBS <libraries...>)
#
# Create the shared library glGrib-<name> (SONAME libglGrib-<name>.so.0) from
# the common sources, compiled with -DGLGRIB_USE_<NAME>, and the program
# glgrib-<name>. If issue #7 (single build with both backends) is done, this
# is the only place to change.

function (glgrib_add_backend name)
  cmake_parse_arguments (ARG "" "" "SOURCES;LIBS;DEFINITIONS" ${ARGN})

  string (TOUPPER "${name}" NAME)

  set (lib glGrib-${name})

  add_library (${lib} SHARED ${ARG_SOURCES})
  set_target_properties (${lib} PROPERTIES VERSION 0 SOVERSION 0)

  # Headers (Options.h, OpenGL.h, ...) depend on the backend: the definition
  # is public, so that programs and bindings see the same declarations
  target_compile_definitions (${lib} PUBLIC GLGRIB_USE_${NAME} ${ARG_DEFINITIONS})
  if (GLGRIB_CHECK_BOUND)
    target_compile_definitions (${lib} PUBLIC GLGRIB_CHECK_BOUND)
  endif ()

  target_compile_options (${lib} PRIVATE -Wall)

  target_include_directories (${lib}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src/glGrib
  )

  target_link_libraries (${lib}
    PUBLIC
      OpenMP::OpenMP_CXX
    PRIVATE
      LFI
      ${ARG_LIBS}
      PNG::PNG
      PkgConfig::READLINE
      OpenSSL::SSL OpenSSL::Crypto
      Threads::Threads
      ${GLGRIB_SQLITE3}
      CURL::libcurl
      PkgConfig::SHAPELIB
      glgrib::eccodes
      PkgConfig::NETCDF_CXX4
  )

  install (TARGETS ${lib}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            NAMELINK_COMPONENT dev
  )

  # Program

  set (exe glgrib-${name})

  add_executable (${exe} ${CMAKE_SOURCE_DIR}/src/glGrib/glGrib.cc)
  target_compile_options (${exe} PRIVATE -Wall)
  target_link_libraries (${exe} PRIVATE ${lib})

  install (TARGETS ${exe} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

  glgrib_add_manpage (${exe} ${exe})
endfunction ()

# glgrib_add_manpage (<page name> <program target>)
#
# Run the program with --pod in the build tree and convert the output with
# pod2man into share/man/man1/<page name>.1 (not compressed: dh_compress or
# the user's man setup takes care of that).

set (GLGRIB_MAN1_DIR ${CMAKE_BINARY_DIR}/share/man/man1)
set (GLGRIB_MANPAGE_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/GlgribManPage.cmake)

function (glgrib_add_manpage page program)
  if (NOT GLGRIB_WITH_MANPAGES)
    return ()
  endif ()

  set (out ${GLGRIB_MAN1_DIR}/${page}.1)

  add_custom_command (
    OUTPUT  ${out}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GLGRIB_MAN1_DIR}
    COMMAND ${CMAKE_COMMAND}
            -DPROGRAM=$<TARGET_FILE:${program}> -DPAGE=${page}
            -DPOD2MAN=${POD2MAN} -DOUTPUT=${out}
            -P ${GLGRIB_MANPAGE_SCRIPT}
    DEPENDS ${program} ${GLGRIB_MANPAGE_SCRIPT}
    COMMENT "Generating man page ${page}.1"
    VERBATIM
  )

  add_custom_target (man-${page} ALL DEPENDS ${out})

  install (FILES ${out} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endfunction ()
