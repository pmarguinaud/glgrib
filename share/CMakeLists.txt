# Data

set (data_src ${CMAKE_SOURCE_DIR}/share/glgrib)
set (data_bin ${CMAKE_BINARY_DIR}/share/glgrib)

set (data_entries glGrib.db coastlines fonts landscape testdata)

# Build tree: symlinks to the source data, so that programs run from
# <build>/bin find them (the shaders are generated there by src/glGrib)

file (MAKE_DIRECTORY ${data_bin})
foreach (entry ${data_entries})
  if (NOT EXISTS ${data_bin}/${entry})
    file (CREATE_LINK ${data_src}/${entry} ${data_bin}/${entry} SYMBOLIC)
  endif ()
endforeach ()

install (FILES ${data_src}/glGrib.db DESTINATION ${CMAKE_INSTALL_DATADIR}/glgrib)
install (DIRECTORY ${data_src}/coastlines ${data_src}/fonts ${data_src}/landscape ${data_src}/testdata
  DESTINATION ${CMAKE_INSTALL_DATADIR}/glgrib
  USE_SOURCE_PERMISSIONS
)

# Headers

install (DIRECTORY ${CMAKE_SOURCE_DIR}/include/glGrib
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Documentation

install (DIRECTORY ${CMAKE_SOURCE_DIR}/share/doc/glgrib
  DESTINATION ${CMAKE_INSTALL_DATADIR}/doc
  USE_SOURCE_PERMISSIONS
)
