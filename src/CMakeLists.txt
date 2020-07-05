
# glGribDBase2SQLite.cc

set (SRC
   glGribGrok.cc
   glGribDBase.cc
   glGribShellInterpreter.cc
   glGribShellRegular.cc
   glGribContainer.cc
   glGribWindowDiffSet.cc
   glGribWindowSet.cc
   glGribTicks.cc
   glGribFitPolynomial.cc
   glGribFieldIsoFill.cc
   glGribSubdivide.cc
   glGribEarCut.cc
   glGribLand.cc
   glGribSQLite.cc
   glGribTest.cc
   glGribCities.cc
   glGribPoints.cc
   glGribShapeLib.cc
   glGribDepartements.cc
   glGribMapscale.cc
   glGribRivers.cc
   glGribBorder.cc
   glGribLines.cc
   glGribGSHHG.cc
   glGribImage.cc
   glGribResolve.cc
   glGribFieldContour.cc
   glGribFieldVector.cc
   glGribFieldScalar.cc
   glGribGeometryLambert.cc
   glGribProjection.cc
   glGribColorbar.cc
   glGribFont.cc
   glGribString.cc
   glGribGeometryLatLon.cc
   glGribWindowOffscreen.cc
   glGribGeometry.cc
   glGribGeometryGaussian.cc
   glGribWindow.cc
   glGribOptions.cc
   glGribShell.cc
   glGribBitmap.cc
   glGribLandScape.cc
   glGribPalette.cc
   glGribField.cc
   glGribLoader.cc
   glGribPolygon.cc
   glGribProgram.cc
   glGribView.cc
   glGribOpenGL.cc
   glGribPng.cc
   glGribScene.cc
   glGribCoast.cc
   glGribGrid.cc
   glGribShader.cc
   glGribFieldStream.cc
   glGribWindowReviewSet.cc
   glGribGeoPoints.cc
   glGribBatch.cc
   glGribRender.cc
)


set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

if (GLGRIB_USE_EGL)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLGRIB_USE_EGL")
else ()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLGRIB_USE_GLFW")
endif ()

add_subdirectory (lfi)
add_library (glGrib SHARED ${SRC})

include_directories (lfi)

add_executable (glgrib glGrib.cc)

target_link_libraries (glgrib 
                       PUBLIC glGrib)

if (GLGRIB_USE_EGL)
target_link_libraries (glgrib
                       PUBLIC EGL PUBLIC gbm)
else ()
target_link_libraries (glgrib
                       PUBLIC glfw PUBLIC GLEW)
endif ()

target_link_libraries (glgrib 
                       PUBLIC eccodes PUBLIC GL PUBLIC sqlite3
                       PUBLIC readline PUBLIC png PUBLIC shp PUBLIC ${OPENSSL_SSL_LIBRARY} 
                       PUBLIC ${OPENSSL_CRYPTO_LIBRARY} PUBLIC OpenMP::OpenMP_CXX PUBLIC LFI
                       PUBLIC ${CURL_LIBRARIES} PUBLIC netcdf_c++4 netcdf)

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

