

FLAGS=-std=c++11 -g 
FLAGS=-std=c++11 -g -I$(HOME)/install/eccodes-2.12.0/include -I$(HOME)/glgrib/usr/include -L$(HOME)/glgrib/usr/lib64 -Wl,-rpath,$(HOME)/glgrib/usr/lib64 -L$(HOME)/eccodes-2.12.0/lib -Wl,-rpath,$(HOME)/eccodes-2.12.0/lib -leccodes -lGLEW -lGL -lEGL -lglfw -lpng

glgrib.x: glgrib_palette.o glgrib_field.o glgrib_landscape.o glgrib_coords_world.o glgrib_coords.o glgrib_cube2.o glgrib_load.o glgrib_polygon.o glgrib_program.o glgrib_view.o glgrib_polyhedron.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_fb.o glgrib_world.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_cube.o glgrib_cube1.o glgrib_x11.o glgrib_shader.o
	g++  $(FLAGS) -o glgrib.x glgrib_palette.o glgrib_field.o glgrib_landscape.o glgrib_coords_world.o glgrib_coords.o glgrib_cube2.o glgrib_load.o glgrib_polygon.o glgrib_view.o glgrib_program.o glgrib_polyhedron.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_fb.o glgrib_world.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_cube.o glgrib_cube1.o glgrib_x11.o  glgrib_shader.o 

%.o: %.cc
	g++ $(FLAGS) -o $@ -c $<

clean: 
	\rm -f *.o *.x

view1.x: view1.cc
	g++ $(FLAGS) -g -o view1.x view1.cc

view.x: view.cc
	g++ $(FLAGS) -g -o view.x view.cc

glwhat.x: glwhat.cc
	g++ $(FLAGS) -g -o glwhat.x glwhat.cc

