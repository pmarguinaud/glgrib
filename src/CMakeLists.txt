
# glGribDBase2SQLite.cc

set (SRC
  glGribWindowReviewSet.cc
  glGribBitmap.cc
  glGribBorder.cc
  glGribCities.cc
  glGribCoast.cc
  glGribColorbar.cc
  glGribContainer.cc
  glGribDBase.cc
  glGribDepartements.cc
  glGribEarCut.cc
  glGribField.cc
  glGribFieldContour.cc
  glGribFieldFloatBuffer.cc
  glGribFieldIsoFill.cc
  glGribFieldScalar.cc
  glGribFieldStream.cc
  glGribFieldVector.cc
  glGribFitPolynomial.cc
  glGribFont.cc
  glGribGeometry.cc
  glGribGeometryGaussian.cc
  glGribGeometryLambert.cc
  glGribGeometryLatLon.cc
  glGribGrid.cc
  glGribGSHHG.cc
  glGribImage.cc
  glGribLand.cc
  glGribLandScape.cc
  glGribLines.cc
  glGribLoader.cc
  glGribMapscale.cc
  glGribOpenGL.cc
  glGribOptions.cc
  glGribPalette.cc
  glGribPng.cc
  glGribPoints.cc
  glGribPolygon.cc
  glGribProgram.cc
  glGribProjection.cc
  glGribResolve.cc
  glGribRivers.cc
  glGribScene.cc
  glGribShader.cc
  glGribShapeLib.cc
  glGribShell.cc
  glGribShellInterpreter.cc
  glGribShellRegular.cc
  glGribSQLite.cc
  glGribString.cc
  glGribSubdivide.cc
  glGribTest.cc
  glGribTicks.cc
  glGribView.cc
  glGribWindow.cc
  glGribWindowDiffSet.cc
  glGribWindowOffscreen.cc
  glGribWindowSet.cc
  glGribGeoPoints.cc
)

add_subdirectory (lfi)
add_library (glGrib SHARED ${SRC})


add_executable (glgrib glGrib.cc)

target_link_libraries (glgrib 
                       PUBLIC glGrib 
                       PUBLIC eccodes PUBLIC glfw PUBLIC GLEW PUBLIC GL PUBLIC sqlite3
                       PUBLIC readline PUBLIC png PUBLIC shp PUBLIC ${OPENSSL_SSL_LIBRARY} 
                       PUBLIC ${OPENSSL_CRYPTO_LIBRARY} PUBLIC OpenMP::OpenMP_CXX PUBLIC LFI
                       PUBLIC ${NETCDF_CXX_LIBRARIES} PUBLIC ${CURL_LIBRARIES})

set_target_properties (glgrib PROPERTIES 
                       INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

get_target_property (LIBS glgrib LINK_LIBRARIES)

if (Python_FOUND)
  file (COPY python DESTINATION ${CMAKE_BINARY_DIR})
  configure_file (python/setup.py.in ${CMAKE_BINARY_DIR}/python/setup.py @ONLY)
endif ()

if (Perl_FOUND)
  file (COPY perl DESTINATION ${CMAKE_BINARY_DIR})
  configure_file (perl/Makefile.PL.in ${CMAKE_BINARY_DIR}/perl/Makefile.PL @ONLY)
endif ()

install (TARGETS glgrib DESTINATION bin)
install (TARGETS glGrib DESTINATION lib)

