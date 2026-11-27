cmake_minimum_required (VERSION 3.18)

# Release: -O2 as the makefiles, without -DNDEBUG (keep the assertions)
set (CMAKE_C_FLAGS_RELEASE   "-O2" CACHE STRING "C flags of Release builds")
set (CMAKE_CXX_FLAGS_RELEASE "-O2" CACHE STRING "C++ flags of Release builds")

# Link libGL (as the makefiles), not GLVND's libOpenGL + libGLX
set (OpenGL_GL_PREFERENCE LEGACY)

project (glgrib VERSION 1.0 LANGUAGES C CXX)

# Options

option (GLGRIB_WITH_GLFW    "Build the GLFW backend (libglGrib-glfw, glgrib-glfw, glgrib)" ON)
option (GLGRIB_WITH_EGL     "Build the EGL backend (libglGrib-egl, glgrib-egl)"            ON)
option (GLGRIB_WITH_GBM     "Add GBM support to the EGL backend"                           OFF)
option (GLGRIB_WITH_PERL    "Build the Perl XS module glGrib::glfw and install glgrib-tk"  ON)
option (GLGRIB_WITH_PYTHON  "Build the Python extension glGrib.glfw"                       ON)
option (GLGRIB_WITH_TOOLS   "Build gshhs_clean and glGribDBase2SQLite (not installed)"     OFF)
option (GLGRIB_WITH_MANPAGES "Generate man pages by running the programs with --pod"       ON)
option (GLGRIB_CHECK_BOUND  "Enable bound checking (debug)"                                OFF)

set (GLGRIB_PYTHON_INSTALL_DIR "" CACHE PATH
     "Install directory of the Python extension (default: from sysconfig)")

if ((NOT GLGRIB_WITH_GLFW) AND (NOT GLGRIB_WITH_EGL))
  message (FATAL_ERROR "At least one of GLGRIB_WITH_GLFW and GLGRIB_WITH_EGL must be ON")
endif ()

if ((NOT GLGRIB_WITH_GLFW) AND (GLGRIB_WITH_PERL OR GLGRIB_WITH_PYTHON))
  message (FATAL_ERROR "The Perl and Python bindings need GLGRIB_WITH_GLFW")
endif ()

# Settings

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set (CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

include (GNUInstallDirs)

# Same layout as the source tree, so that programs run from the build tree
# find their data in <build>/share/glgrib (see Resolve.cc)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Installed programs find their libraries in the standard directories;
# outside the standard directories, add an rpath

list (FIND CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_FULL_LIBDIR}" _glgrib_system_libdir)
if (_glgrib_system_libdir EQUAL -1)
  set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
endif ()

# Dependencies

find_package (PkgConfig REQUIRED)
find_package (OpenMP REQUIRED COMPONENTS C CXX)
find_package (PNG REQUIRED)
find_package (CURL REQUIRED)
find_package (SQLite3 REQUIRED)
# CMake >= 4.3 names the target SQLite3::SQLite3, older versions SQLite::SQLite3
if (TARGET SQLite3::SQLite3)
  set (GLGRIB_SQLITE3 SQLite3::SQLite3)
else ()
  set (GLGRIB_SQLITE3 SQLite::SQLite3)
endif ()
find_package (OpenSSL REQUIRED)
find_package (Threads REQUIRED)

# ecCodes: its pkg-config file and CMake package add an rpath or pull in
# Fortran; look for the C library only
find_path (ECCODES_INCLUDE_DIR eccodes.h)
find_library (ECCODES_LIBRARY eccodes)
if ((NOT ECCODES_INCLUDE_DIR) OR (NOT ECCODES_LIBRARY))
  message (FATAL_ERROR "ecCodes not found (Debian: libeccodes-dev)")
endif ()
add_library (glgrib::eccodes UNKNOWN IMPORTED)
set_target_properties (glgrib::eccodes PROPERTIES
  IMPORTED_LOCATION             "${ECCODES_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${ECCODES_INCLUDE_DIR}"
)

pkg_check_modules (NETCDF_CXX4 REQUIRED IMPORTED_TARGET netcdf-cxx4)
pkg_check_modules (SHAPELIB    REQUIRED IMPORTED_TARGET shapelib)
pkg_check_modules (READLINE    REQUIRED IMPORTED_TARGET readline)

include (CheckIncludeFileCXX)
check_include_file_cxx (glm/glm.hpp GLGRIB_HAVE_GLM)
if (NOT GLGRIB_HAVE_GLM)
  message (FATAL_ERROR "glm headers not found (Debian: libglm-dev)")
endif ()

if (GLGRIB_WITH_GLFW)
  find_package (OpenGL REQUIRED COMPONENTS OpenGL)
  find_package (GLEW REQUIRED)
  find_package (glfw3 3.3 REQUIRED)
endif ()

if (GLGRIB_WITH_EGL)
  find_package (OpenGL REQUIRED COMPONENTS OpenGL EGL)
  if (GLGRIB_WITH_GBM)
    pkg_check_modules (GBM REQUIRED IMPORTED_TARGET gbm)
  endif ()
endif ()

find_package (Perl REQUIRED)

find_program (POD2MAN pod2man REQUIRED)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include (GlgribHelpers)

# Subdirectories

add_subdirectory (src/LFI)
add_subdirectory (src/glGrib)

if (GLGRIB_WITH_PERL)
  add_subdirectory (src/perl5)
  add_subdirectory (src/glGribTk)
endif ()

if (GLGRIB_WITH_PYTHON)
  add_subdirectory (src/python3)
endif ()

if (GLGRIB_WITH_TOOLS)
  add_subdirectory (src/gshhsClean)
endif ()

add_subdirectory (share)
