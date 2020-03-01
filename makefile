

ECCODES_PREFIX=$(HOME)/install/eccodes-2.14.0

LDFLAGS=-fopenmp -lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto -lpthread -lsqlite3 -Llfi -llfi -lcurl -lshp -L$(ECCODES_PREFIX)/lib -Wl,-rpath,$(ECCODES_PREFIX)/lib -leccodes -L$(HOME)/3d/usr/lib64 -Wl,-rpath,$(HOME)/3d/usr/lib64 -L$(HOME)/3d/usr/lib -Wl,-rpath,$(HOME)/3d/usr/lib
RUNTEST=./runtest.pl $@ ./glGrib.x


CXXFLAGS=-O2 -fopenmp -std=c++11 -g -I$(HOME)/3d/usr/include -I$(ECCODES_PREFIX)/include  

all: glGrib.x 
	@./shaders/expand.pl

OBJECTS=glGribContainer.o glGribWindowDiffSet.o glGribWindowSet.o glGribTicks.o glGribFitpolynomial.o glGribFieldIsofill.o glGribSubdivide.o glGribEarcut.o glGribLand.o glGribSqlite.o glGribDbase.o glGribTest.o glGribCities.o glGribPoints.o glGribShapelib.o glGribDepartements.o glGribMapscale.o glGribRivers.o glGribBorder.o glGribLines.o glGribGshhg.o glGribImage.o glGribResolve.o glGribFieldFloatBuffer.o glGribFieldContour.o glGribFieldVector.o glGribFieldScalar.o glGribGeometryLambert.o glGribProjection.o glGribColorbar.o glGribFont.o glGribString.o glGribGeometryLatlon.o glGribWindowOffscreen.o glGribGeometry.o glGribGeometryGaussian.o glGribWindow.o glGribOptions.o glGribShell.o glGribBitmap.o glGribLandscape.o glGribPalette.o glGribField.o glGribLoader.o glGribPolygon.o glGribProgram.o glGribView.o glGribWorld.o glGribOpengl.o glGribPng.o glGribScene.o glGribCoast.o glGribGrid.o glGribShader.o glGribFieldStream.o

glGrib.x: $(OBJECTS) glGrib.cc
	g++ $(CXXFLAGS) -o glGrib.x glGrib.cc $(OBJECTS) $(LDFLAGS) 

glGribTestit.x: $(OBJECTS) glGribTestit.cc
	g++ $(CXXFLAGS) -o glGribTestit.x glGribTestit.cc $(OBJECTS) $(LDFLAGS) 

glGribDbase2sqlite.x: glGribDbase2sqlite.o glGribDbase.o
	g++ $(CXXFLAGS) -o glGribDbase2sqlite.x glGribDbase2sqlite.o glGribDbase.o -lsqlite3

%.o: %.cc
	g++ $(CXXFLAGS) -o $@ -c $<

clean: 
	\rm -f *.o *.x

