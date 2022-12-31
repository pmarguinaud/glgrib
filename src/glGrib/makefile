
TOP=../..

include $(TOP)/makefile.inc

ifeq ($(BUILD), )

all:
	$(MAKE) BUILD=glfw
	$(MAKE) BUILD=egl
clean:
	$(MAKE) BUILD=glfw clean
	$(MAKE) BUILD=egl clean

else

SUFFIX=-$(BUILD)
all: $(TOP)/bin/glgrib$(SUFFIX) $(TOP)/lib/libglGrib$(SUFFIX).so.0
	@./shaders/expand.pl $(TOP)

ifeq ($(BUILD), glfw)

all:
	@\ln -sf glgrib$(SUFFIX) $(TOP)/bin/glgrib

endif

OBJECTS=$(BUILD)/Grok.o $(BUILD)/DBase.o $(BUILD)/ShellInterpreter.o $(BUILD)/ShellRegular.o $(BUILD)/Container.o $(BUILD)/WindowDiffSet.o $(BUILD)/WindowSet.o $(BUILD)/Ticks.o $(BUILD)/FitPolynomial.o $(BUILD)/FieldIsoFill.o $(BUILD)/Subdivide.o $(BUILD)/EarCut.o $(BUILD)/Land.o $(BUILD)/SQLite.o $(BUILD)/Test.o $(BUILD)/Cities.o $(BUILD)/Points.o $(BUILD)/ShapeLib.o $(BUILD)/Departements.o $(BUILD)/Mapscale.o $(BUILD)/Rivers.o $(BUILD)/Border.o $(BUILD)/Lines.o $(BUILD)/GSHHG.o $(BUILD)/Image.o $(BUILD)/Resolve.o $(BUILD)/FieldContour.o $(BUILD)/FieldVector.o $(BUILD)/FieldScalar.o $(BUILD)/GeometryLambert.o $(BUILD)/Projection.o $(BUILD)/Colorbar.o $(BUILD)/Font.o $(BUILD)/String.o $(BUILD)/GeometryLatLon.o $(BUILD)/WindowOffscreen.o $(BUILD)/Geometry.o $(BUILD)/GeometryGaussian.o $(BUILD)/Window.o $(BUILD)/Options.o $(BUILD)/Shell.o $(BUILD)/Bitmap.o $(BUILD)/LandScape.o $(BUILD)/Palette.o $(BUILD)/Field.o $(BUILD)/Loader.o $(BUILD)/Polygon.o $(BUILD)/Program.o $(BUILD)/View.o $(BUILD)/OpenGL.o $(BUILD)/Png.o $(BUILD)/Scene.o $(BUILD)/Coast.o $(BUILD)/Grid.o $(BUILD)/Shader.o $(BUILD)/FieldStream.o $(BUILD)/WindowReviewSet.o $(BUILD)/GeoPoints.o $(BUILD)/Batch.o $(BUILD)/Render.o $(BUILD)/MD5.o

$(TOP)/lib/libglGrib$(SUFFIX).so.0: $(OBJECTS)
	g++ -shared  $(RPATH) -Wl,-soname,libglGrib$(SUFFIX).so.0 -o $(TOP)/lib/libglGrib$(SUFFIX).so.0 $(OBJECTS) -L$(TOP)/lib -lLFI $(LIBS_GLGRIB) $(LDFLAGS_USER)
	@\ln -sf libglGrib$(SUFFIX).so.0 $(TOP)/lib/libglGrib$(SUFFIX).so

$(TOP)/bin/glgrib$(SUFFIX): glGrib.cc $(TOP)/lib/libglGrib$(SUFFIX).so.0
	g++ $(CXXFLAGS) -o $(TOP)/bin/glgrib$(SUFFIX) glGrib.cc $(LDFLAGS) -I$(TOP)/include

$(TOP)/bin/glGribDBase2SQLite$(SUFFIX).x: glGribDBase2SQLite.cc $(TOP)/lib/libglGrib$(SUFFIX).so
	g++ $(CXXFLAGS) -o $(BUILD)/glGribDBase2SQLite$(SUFFIX).x glGribDBase2SQLite.cc $(LDFLAGS)

$(BUILD)/%.o: %.cc
	@mkdir -p $(BUILD)
	g++ $(CXXFLAGS) -o $@ -c $< -I$(TOP)/include

clean: 
	\rm -f $(BUILD)/*.o $(BUILD)/*.x $(TOP)/lib/libglGrib$(SUFFIX).so* $(TOP)/bin/glgrib$(SUFFIX) $(TOP)/bin/glGribDBase2SQLite$(SUFFIX).x

endif
