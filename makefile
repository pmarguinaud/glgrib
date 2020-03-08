include makefile.inc

all: glGrib.x 
	@./shaders/expand.pl

OBJECTS=glGribShellInterpreter.o glGribShellRegular.o glGribContainer.o glGribWindowDiffSet.o glGribWindowSet.o glGribTicks.o glGribFitPolynomial.o glGribFieldIsoFill.o glGribSubdivide.o glGribEarCut.o glGribLand.o glGribSQLite.o glGribDBase.o glGribTest.o glGribCities.o glGribPoints.o glGribShapeLib.o glGribDepartements.o glGribMapscale.o glGribRivers.o glGribBorder.o glGribLines.o glGribGSHHG.o glGribImage.o glGribResolve.o glGribFieldFloatBuffer.o glGribFieldContour.o glGribFieldVector.o glGribFieldScalar.o glGribGeometryLambert.o glGribProjection.o glGribColorbar.o glGribFont.o glGribString.o glGribGeometryLatLon.o glGribWindowOffscreen.o glGribGeometry.o glGribGeometryGaussian.o glGribWindow.o glGribOptions.o glGribShell.o glGribBitmap.o glGribLandScape.o glGribPalette.o glGribField.o glGribLoader.o glGribPolygon.o glGribProgram.o glGribView.o glGribWorld.o glGribOpenGL.o glGribPng.o glGribScene.o glGribCoast.o glGribGrid.o glGribShader.o glGribFieldStream.o

libglGrib.so: $(OBJECTS)
	g++ -shared -o libglGrib.so $(OBJECTS)

glGrib.x: glGrib.cc libglGrib.so
	g++ $(CXXFLAGS) -o glGrib.x glGrib.cc -Wl,-rpath,$(HOME)/3d/glgrib -L. $(LDFLAGS) 

glGribDBase2SQLite.x: glGribDBase2SQLite.o glGribDBase.o
	g++ $(CXXFLAGS) -o glGribDBase2SQLite.x glGribDBase2SQLite.o glGribDBase.o $(LDFLAGS)

%.o: %.cc
	g++ $(CXXFLAGS) -o $@ -c $<

clean: 
	\rm -f *.o *.x *.so

