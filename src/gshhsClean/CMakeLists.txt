# gshhs_clean: offline cleaning of coastline shapefiles (GSHHS, Natural Earth);
# built with GLGRIB_WITH_TOOLS, not installed

add_executable (gshhs_clean gshhs_clean.cc)
target_compile_options (gshhs_clean PRIVATE -Wall)
target_include_directories (gshhs_clean PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries (gshhs_clean PRIVATE PkgConfig::SHAPELIB OpenMP::OpenMP_CXX)
