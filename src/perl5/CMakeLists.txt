# Perl XS module glGrib::glfw, built without ExtUtils::MakeMaker

# Perl configuration

function (glgrib_perl_config var key)
  execute_process (
    COMMAND ${PERL_EXECUTABLE} -MConfig -e "print \$Config{${key}}"
    OUTPUT_VARIABLE value
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
  set (${var} "${value}" PARENT_SCOPE)
endfunction ()

glgrib_perl_config (PERL_ARCHLIB    archlibexp)
glgrib_perl_config (PERL_PRIVLIB    privlibexp)
glgrib_perl_config (PERL_CCFLAGS    ccflags)
glgrib_perl_config (PERL_VENDORARCH installvendorarch)
glgrib_perl_config (PERL_VENDORLIB  installvendorlib)

# Install directories: Perl's vendor directories when installing in /usr,
# <prefix>/lib/perl5 otherwise. The default is computed at each configure
# (the prefix may change); a value given with -D is kept.
set (GLGRIB_PERL_ARCH_INSTALL_DIR "" CACHE PATH "Install directory of Perl XS modules (default: from the prefix)")
set (GLGRIB_PERL_LIB_INSTALL_DIR  "" CACHE PATH "Install directory of pure Perl modules (default: from the prefix)")

if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
  set (perl_arch_dir ${PERL_VENDORARCH})
  set (perl_lib_dir  ${PERL_VENDORLIB})
else ()
  set (perl_arch_dir ${CMAKE_INSTALL_LIBDIR}/perl5)
  set (perl_lib_dir  ${CMAKE_INSTALL_LIBDIR}/perl5)
endif ()

if (NOT GLGRIB_PERL_ARCH_INSTALL_DIR STREQUAL "")
  set (perl_arch_dir ${GLGRIB_PERL_ARCH_INSTALL_DIR})
endif ()
if (NOT GLGRIB_PERL_LIB_INSTALL_DIR STREQUAL "")
  set (perl_lib_dir ${GLGRIB_PERL_LIB_INSTALL_DIR})
endif ()

set (GLGRIB_PERL_LIB_DIR ${perl_lib_dir} PARENT_SCOPE)

find_program (XSUBPP xsubpp REQUIRED)

if (NOT EXISTS ${PERL_ARCHLIB}/CORE/perl.h)
  message (FATAL_ERROR "perl.h not found in ${PERL_ARCHLIB}/CORE (Debian: libperl-dev)")
endif ()

# glGrib::glfw

set (xs_dir ${CMAKE_CURRENT_SOURCE_DIR}/glfw)
set (xs_c   ${CMAKE_CURRENT_BINARY_DIR}/glfw.cc)

add_custom_command (
  OUTPUT  ${xs_c}
  COMMAND ${XSUBPP} -typemap ${PERL_PRIVLIB}/ExtUtils/typemap -output ${xs_c} ${xs_dir}/glfw.xs
  DEPENDS ${xs_dir}/glfw.xs
  WORKING_DIRECTORY ${xs_dir}
  COMMENT "Generating glfw.cc from glfw.xs"
  VERBATIM
)

add_library (perl_glfw MODULE ${xs_c})

separate_arguments (perl_ccflags UNIX_COMMAND "${PERL_CCFLAGS}")

target_compile_options (perl_glfw PRIVATE ${perl_ccflags})
target_include_directories (perl_glfw PRIVATE ${xs_dir} ${PERL_ARCHLIB}/CORE)
target_compile_definitions (perl_glfw PRIVATE
  GLGRIB_PREFIX="${CMAKE_INSTALL_PREFIX}"
  VERSION="0.01"
  XS_VERSION="0.01"
)
target_link_libraries (perl_glfw PRIVATE glGrib-glfw)

# Build tree: <build>/perl5/{glGrib/glfw.pm,auto/glGrib/glfw/glfw.so},
# usable with perl -I<build>/perl5
set (perl_bin ${CMAKE_BINARY_DIR}/perl5)

set_target_properties (perl_glfw PROPERTIES
  OUTPUT_NAME glfw
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY ${perl_bin}/auto/glGrib/glfw
)

configure_file (${xs_dir}/lib/glGrib/glfw.pm ${perl_bin}/glGrib/glfw.pm COPYONLY)

install (TARGETS perl_glfw
  LIBRARY DESTINATION ${perl_arch_dir}/auto/glGrib/glfw
)
install (FILES ${xs_dir}/lib/glGrib/glfw.pm
  DESTINATION ${perl_arch_dir}/glGrib
)

# Man page glGrib::glfw.3pm
# "::" cannot appear in a make target: generate glGrib__glfw.3pm and rename
# it at install time
set (man3 ${CMAKE_BINARY_DIR}/share/man/man3/glGrib__glfw.3pm)
add_custom_command (
  OUTPUT  ${man3}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/share/man/man3
  COMMAND ${POD2MAN} --section=3pm ${xs_dir}/lib/glGrib/glfw.pm ${man3}
  DEPENDS ${xs_dir}/lib/glGrib/glfw.pm
  COMMENT "Generating man page glGrib::glfw.3pm"
  VERBATIM
)
add_custom_target (man-perl-glfw ALL DEPENDS ${man3})
install (FILES ${man3} DESTINATION ${CMAKE_INSTALL_MANDIR}/man3 RENAME glGrib::glfw.3pm)
