
TOP=..

include $(TOP)/makefile.inc

all: $(TOP)/bin/glGrib.x 
	@./shaders/expand.pl $(TOP)

OBJECTS=glGribDBase.o glGribShellInterpreter.o glGribShellRegular.o glGribContainer.o glGribWindowDiffSet.o glGribWindowSet.o glGribTicks.o glGribFitPolynomial.o glGribFieldIsoFill.o glGribSubdivide.o glGribEarCut.o glGribLand.o glGribSQLite.o glGribTest.o glGribCities.o glGribPoints.o glGribShapeLib.o glGribDepartements.o glGribMapscale.o glGribRivers.o glGribBorder.o glGribLines.o glGribGSHHG.o glGribImage.o glGribResolve.o glGribFieldFloatBuffer.o glGribFieldContour.o glGribFieldVector.o glGribFieldScalar.o glGribGeometryLambert.o glGribProjection.o glGribColorbar.o glGribFont.o glGribString.o glGribGeometryLatLon.o glGribWindowOffscreen.o glGribGeometry.o glGribGeometryGaussian.o glGribWindow.o glGribOptions.o glGribShell.o glGribBitmap.o glGribLandScape.o glGribPalette.o glGribField.o glGribLoader.o glGribPolygon.o glGribProgram.o glGribView.o glGribWorld.o glGribOpenGL.o glGribPng.o glGribScene.o glGribCoast.o glGribGrid.o glGribShader.o glGribFieldStream.o

$(TOP)/lib/libglGrib.so: $(OBJECTS)
	g++ -shared -o $(TOP)/lib/libglGrib.so $(OBJECTS)

$(TOP)/bin/glGrib.x: glGrib.cc $(TOP)/lib/libglGrib.so
	g++ $(CXXFLAGS) -o $(TOP)/bin/glGrib.x glGrib.cc $(LDFLAGS) 

$(TOP)/bin/glGribDBase2SQLite.x: glGribDBase2SQLite.cc $(TOP)/lib/libglGrib.so
	g++ $(CXXFLAGS) -o glGribDBase2SQLite.x glGribDBase2SQLite.cc $(LDFLAGS)

%.o: %.cc
	g++ $(CXXFLAGS) -o $@ -c $<

clean: 
	\rm -f *.o *.x *.so $(TOP)/lib/libglGrib.so $(TOP)/bin/glGrib.x $(TOP)/bin/glGribDBase2SQLite.x

