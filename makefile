

LDFLAGS=-fopenmp -leccodes -lGLEW -lGL -lglfw -lpng -lreadline -ltinfo -lssl -lcrypto -lpthread -lsqlite3
RUNTEST=./runtest.pl $@ ./glgrib.x

CXXFLAGS=-O2 -fopenmp -std=c++11 -g -I$(HOME)/3d/usr/include -L$(HOME)/3d/usr/lib64 -Wl,-rpath,$(HOME)/3d/usr/lib64 -L$(HOME)/3d/usr/lib -Wl,-rpath,$(HOME)/3d/usr/lib -leccodes -lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto
CXXFLAGS += -I$(HOME)/install/eccodes--2.13.0_FIXOMMCODES/include -L$(HOME)/install/eccodes--2.13.0_FIXOMMCODES/lib -Wl,-rpath,$(HOME)/install/eccodes--2.13.0_FIXOMMCODES/lib

all: glgrib.x glgrib_dbase2sqlite.x

OBJECTS=glgrib_dbase.o glgrib_test.o glgrib_cities.o glgrib_points.o glgrib_shapelib.o glgrib_departements.o glgrib_mapscale.o glgrib_rivers.o glgrib_border.o glgrib_lines.o glgrib_gshhg.o glgrib_image.o glgrib_resolve.o glgrib_field_float_buffer.o glgrib_field_contour.o glgrib_field_vector.o glgrib_field_scalar.o glgrib_geometry_lambert.o glgrib_projection.o glgrib_colorbar.o glgrib_font.o glgrib_string.o glgrib_geometry_latlon.o glgrib_window_offscreen.o glgrib_geometry.o glgrib_geometry_gaussian.o glgrib_window.o glgrib_options.o glgrib_shell.o glgrib_bitmap.o glgrib_landscape.o glgrib_palette.o glgrib_field.o glgrib_loader.o glgrib_polygon.o glgrib_program.o glgrib_view.o glgrib_world.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_scene.o glgrib_coast.o glgrib_grid.o glgrib_shader.o 

glgrib.x: $(OBJECTS)
	g++ $(CXXFLAGS) -o glgrib.x $(OBJECTS) $(LDFLAGS) -Llfi -llfi -lcurl -lshp

glgrib_dbase2sqlite.x: glgrib_dbase2sqlite.o glgrib_dbase.o
	g++ $(CXXFLAGS) -o glgrib_dbase2sqlite.x glgrib_dbase2sqlite.o glgrib_dbase.o -lsqlite3

%.o: %.cc
	g++ $(CXXFLAGS) -o $@ -c $<

clean: 
	\rm -f *.o *.x

glwhat.x: glwhat.cc
	g++ $(CXXFLAGS) -g -o glwhat.x glwhat.cc $(LDFLAGS)


test: ./glgrib.x
	$(RUNTEST)

