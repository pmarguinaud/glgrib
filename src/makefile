
TOP=..

include $(TOP)/makefile.inc

all: $(TOP)/bin/glgrib$(SUFFIX) $(TOP)/lib/libglGrib$(SUFFIX).so.0
	@./shaders/expand.pl $(TOP)

OBJECTS=glGribGrok.o glGribDBase.o glGribShellInterpreter.o glGribShellRegular.o glGribContainer.o glGribWindowDiffSet.o glGribWindowSet.o glGribTicks.o glGribFitPolynomial.o glGribFieldIsoFill.o glGribSubdivide.o glGribEarCut.o glGribLand.o glGribSQLite.o glGribTest.o glGribCities.o glGribPoints.o glGribShapeLib.o glGribDepartements.o glGribMapscale.o glGribRivers.o glGribBorder.o glGribLines.o glGribGSHHG.o glGribImage.o glGribResolve.o glGribFieldContour.o glGribFieldVector.o glGribFieldScalar.o glGribGeometryLambert.o glGribProjection.o glGribColorbar.o glGribFont.o glGribString.o glGribGeometryLatLon.o glGribWindowOffscreen.o glGribGeometry.o glGribGeometryGaussian.o glGribWindow.o glGribOptions.o glGribShell.o glGribBitmap.o glGribLandScape.o glGribPalette.o glGribField.o glGribLoader.o glGribPolygon.o glGribProgram.o glGribView.o glGribOpenGL.o glGribPng.o glGribScene.o glGribCoast.o glGribGrid.o glGribShader.o glGribFieldStream.o glGribWindowReviewSet.o glGribGeoPoints.o glGribBatch.o glGribRender.o glGribMD5.o

$(TOP)/lib/libglGrib$(SUFFIX).so.0: $(OBJECTS)
	g++ -shared  -Wl,-soname,libglGrib$(SUFFIX).so.0 -o $(TOP)/lib/libglGrib$(SUFFIX).so.0 $(OBJECTS)
	@\ln -sf libglGrib$(SUFFIX).so.0 $(TOP)/lib/libglGrib$(SUFFIX).so

$(TOP)/bin/glgrib$(SUFFIX): glGrib.cc $(TOP)/lib/libglGrib$(SUFFIX).so.0
	g++ $(CXXFLAGS) -o $(TOP)/bin/glgrib$(SUFFIX) glGrib.cc $(LDFLAGS) 

$(TOP)/bin/glGribDBase2SQLite$(SUFFIX).x: glGribDBase2SQLite.cc $(TOP)/lib/libglGrib$(SUFFIX).so
	g++ $(CXXFLAGS) -o glGribDBase2SQLite$(SUFFIX).x glGribDBase2SQLite.cc $(LDFLAGS)

%.o: %.cc
	g++ $(CXXFLAGS) -o $@ -c $< -I$(TOP)/src/lfi

clean: 
	\rm -f *.o *.x *.so $(TOP)/lib/libglGrib$(SUFFIX).so $(TOP)/bin/glgrib$(SUFFIX) $(TOP)/bin/glGribDBase2SQLite$(SUFFIX).x

