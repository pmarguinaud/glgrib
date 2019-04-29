

CXXFLAGS=-std=c++11 -g -fopenmp
LDFLAGS=-fopenmp -leccodes -lGLEW -lGL -lEGL -lglfw -lpng -lreadline -ltinfo

CXXFLAGS=-fopenmp -std=c++11 -g -I$(HOME)/install/eccodes-2.12.0/include -I$(HOME)/glgrib/usr/include -L$(HOME)/glgrib/usr/lib64 -Wl,-rpath,$(HOME)/glgrib/usr/lib64 -L$(HOME)/eccodes-2.12.0/lib -Wl,-rpath,$(HOME)/eccodes-2.12.0/lib -leccodes -lGLEW -lGL -lEGL -lglfw -lpng -lreadline -lncurses -ltinfo

glgrib.x: glgrib_geometry.o glgrib_geometry_gaussian.o glgrib_window.o glgrib_options.o glgrib_shell.o glgrib_landscape_rgb.o glgrib_bmp.o glgrib_landscape_tex.o glgrib_palette.o glgrib_field.o glgrib_landscape.o glgrib_coords_world.o glgrib_coords.o glgrib_load.o glgrib_polygon.o glgrib_program.o glgrib_view.o glgrib_polyhedron.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_fb.o glgrib_world.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_x11.o glgrib_shader.o
	g++  $(CXXFLAGS) -o glgrib.x glgrib_geometry.o glgrib_geometry_gaussian.o glgrib_window.o glgrib_options.o glgrib_shell.o glgrib_landscape_rgb.o glgrib_bmp.o glgrib_landscape_tex.o glgrib_palette.o glgrib_field.o glgrib_landscape.o glgrib_coords_world.o glgrib_coords.o glgrib_load.o glgrib_polygon.o glgrib_view.o glgrib_program.o glgrib_polyhedron.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_fb.o glgrib_world.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_x11.o  glgrib_shader.o $(LDFLAGS)

%.o: %.cc
	g++ $(CXXFLAGS) -o $@ -c $<

clean: 
	\rm -f *.o *.x

view1.x: view1.cc
	g++ $(CXXFLAGS) -g -o view1.x view1.cc $(LDFLAGS)

view.x: view.cc
	g++ $(CXXFLAGS) -g -o view.x view.cc $(LDFLAGS)

glwhat.x: glwhat.cc
	g++ $(CXXFLAGS) -g -o glwhat.x glwhat.cc $(LDFLAGS)

