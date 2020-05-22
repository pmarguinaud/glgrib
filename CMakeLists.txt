cmake_minimum_required (VERSION 3.16.3 FATAL_ERROR)

project (glGrib VERSION 0.5 LANGUAGES C CXX)

set (CMAKE_CXX_STANDARD 11) 
set (CMAKE_CXX_STANDARD_REQUIRED ON) 

set (OpenGL_GL_PREFERENCE LEGACY) # Or GLVND

find_package (OpenMP REQUIRED)
find_package (glfw3 3.2 REQUIRED)
find_package (GLEW 2.0 REQUIRED)
find_package (eccodes 2.14.0 REQUIRED)
find_package (OpenGL 1.2 REQUIRED)
find_package (SQLite3 3.22.0 REQUIRED)
find_package (PNG 1.6.34 REQUIRED)
find_package (OpenSSL 1.1.0 REQUIRED)
find_package (CURL REQUIRED)
find_package (netCDF REQUIRED COMPONENTS CXX)
find_package (Python COMPONENTS Interpreter Development)
find_package (Perl COMPONENTS Interpreter Development)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory (src)

install (DIRECTORY share DESTINATION .)
install (PROGRAMS scripts/runtest.pl DESTINATION scripts)

add_custom_target (data ALL)

add_custom_command (TARGET data POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                            ${CMAKE_SOURCE_DIR}/scripts/runtest.pl
                            ${CMAKE_CURRENT_BINARY_DIR}/scripts/runtest.pl)

add_custom_command (TARGET data POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E create_symlink
                            ${CMAKE_SOURCE_DIR}/share
                            ${CMAKE_CURRENT_BINARY_DIR}/share)


                    


