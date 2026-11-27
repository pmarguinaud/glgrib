# Python extension glGrib.glfw

find_package (Python3 REQUIRED COMPONENTS Interpreter Development.Module)

if (GLGRIB_PYTHON_INSTALL_DIR STREQUAL "")
  set (GLGRIB_PYTHON_INSTALL_DIR ${Python3_SITEARCH})
endif ()

Python3_add_library (python_glfw MODULE WITH_SOABI glfw/glfw.cc)

target_compile_definitions (python_glfw PRIVATE GLGRIB_PREFIX="${CMAKE_INSTALL_PREFIX}")
target_link_libraries (python_glfw PRIVATE glGrib-glfw)

# Build tree: <build>/python3/glGrib/glfw.<soabi>.so, usable with
# PYTHONPATH=<build>/python3
set_target_properties (python_glfw PROPERTIES
  OUTPUT_NAME glfw
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python3/glGrib
)

install (TARGETS python_glfw
  LIBRARY DESTINATION ${GLGRIB_PYTHON_INSTALL_DIR}/glGrib
)
