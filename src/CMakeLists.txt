
cmake_minimum_required (VERSION 3.0 FATAL_ERROR)

project (glGrib C CXX)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# glGribDBase2SQLite.cc

set (SRC
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
  glGribWorld.cc
)

add_subdirectory (lfi)

find_package (OpenMP REQUIRED)
find_package (glfw3 3.3 REQUIRED)
find_package (GLEW 2.1 REQUIRED)
find_package (eccodes 2.14.0 REQUIRED)
find_package (OpenGL 1.2 REQUIRED)
find_package (SQLite3 3.28.0 REQUIRED)
find_package (PNG 1.6.37 REQUIRED)
find_package (OpenSSL 1.1.0 REQUIRED)
find_package (CURL REQUIRED)




#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

add_executable (glGrib.x glGrib.cc ${SRC})

target_link_libraries (glGrib.x 
   PUBLIC eccodes PUBLIC glfw PUBLIC GLEW PUBLIC GL PUBLIC ${SQLite3_LIBRARIES} 
   PUBLIC readline PUBLIC png PUBLIC shp PUBLIC ${OPENSSL_SSL_LIBRARY} 
   PUBLIC ${OPENSSL_CRYPTO_LIBRARY} PUBLIC OpenMP::OpenMP_CXX PUBLIC LFI
   PUBLIC ${CURL_LIBRARIES})



