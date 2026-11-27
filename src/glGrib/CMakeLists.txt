set (GLGRIB_SOURCES
  Batch.cc Bitmap.cc Border.cc Cities.cc Coast.cc Colorbar.cc
  ConstrainedDelaunay.cc Container.cc DBase.cc Departements.cc Field.cc
  FieldContour.cc FieldIsoFill.cc FieldScalar.cc FieldStream.cc FieldVector.cc
  FieldVertical.cc FitPolynomial.cc Font.cc GSHHG.cc GeoPoints.cc Geometry.cc
  GeometryGaussian.cc GeometryLambert.cc GeometryLatLon.cc Grid.cc Grok.cc
  Image.cc KdTree.cc Land.cc LandScape.cc Lines.cc Loader.cc MD5.cc Mapscale.cc
  OpenGL.cc Options.cc Palette.cc Png.cc Points.cc Polygon.cc Program.cc
  Projection.cc Render.cc Resolve.cc Rivers.cc SQLite.cc Scene.cc Shader.cc
  ShapeLib.cc Shell.cc ShellInterpreter.cc ShellRegular.cc String.cc
  Subdivide.cc Test.cc Ticks.cc View.cc Window.cc WindowDiffSet.cc
  WindowOffscreen.cc WindowReviewSet.cc WindowSet.cc
)

# Backends

if (GLGRIB_WITH_GLFW)
  glgrib_add_backend (glfw
    SOURCES ${GLGRIB_SOURCES}
    LIBS    GLEW::GLEW glfw OpenGL::GL
  )

  # glgrib is a link to glgrib-glfw, with its own man page
  add_custom_command (
    TARGET glgrib-glfw POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink glgrib-glfw glgrib
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    VERBATIM
  )
  install (FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/glgrib
    DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  glgrib_add_manpage (glgrib glgrib-glfw)
endif ()

if (GLGRIB_WITH_EGL)
  set (egl_libs OpenGL::EGL OpenGL::GL)
  set (egl_defs)
  if (GLGRIB_WITH_GBM)
    list (APPEND egl_libs PkgConfig::GBM)
    list (APPEND egl_defs GLGRIB_USE_GBM)
  endif ()
  glgrib_add_backend (egl
    SOURCES     ${GLGRIB_SOURCES}
    LIBS        ${egl_libs}
    DEFINITIONS ${egl_defs}
  )
endif ()

if (GLGRIB_WITH_TOOLS)
  # Converts DBase files to SQLite; not installed
  if (GLGRIB_WITH_GLFW)
    set (tool_lib glGrib-glfw)
  else ()
    set (tool_lib glGrib-egl)
  endif ()
  add_executable (glGribDBase2SQLite glGribDBase2SQLite.cc)
  target_link_libraries (glGribDBase2SQLite PRIVATE ${tool_lib} PkgConfig::SHAPELIB ${GLGRIB_SQLITE3})
endif ()

# Shaders: expand.pl inlines the #include directives of shaders/src/*.{vs,fs,gs}
# into <build>/share/glgrib/shaders

file (GLOB shader_sources CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/*.vs
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/*.fs
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/src/*.gs
)
file (GLOB_RECURSE shader_includes CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/include/*
)

set (shader_dir ${CMAKE_BINARY_DIR}/share/glgrib/shaders)
set (shader_outputs)
foreach (src ${shader_sources})
  get_filename_component (name ${src} NAME)
  list (APPEND shader_outputs ${shader_dir}/${name})
endforeach ()

add_custom_command (
  OUTPUT  ${shader_outputs}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${shader_dir}
  COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/shaders/expand.pl ${CMAKE_BINARY_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/expand.pl ${shader_sources} ${shader_includes}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Expanding shaders"
  VERBATIM
)

add_custom_target (shaders ALL DEPENDS ${shader_outputs})

install (DIRECTORY ${shader_dir} DESTINATION ${CMAKE_INSTALL_DATADIR}/glgrib)
