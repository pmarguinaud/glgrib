

FLAGS=-std=c++11 -g 

glgrib.x: glgrib_load.o glgrib_polygon.o glgrib_program.o glgrib_view.o glgrib_polyhedron.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_fb.o glgrib_world.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_cube.o glgrib_cube1.o glgrib_x11.o glgrib_shader.o
	g++  $(FLAGS) -o glgrib.x glgrib_load.o glgrib_polygon.o glgrib_view.o glgrib_program.o glgrib_polyhedron.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_fb.o glgrib_world.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_cube.o glgrib_cube1.o glgrib_x11.o  glgrib_shader.o -leccodes -lGLEW -lGL -lEGL -lglfw -lpng

%.o: %.cc
	g++ $(FLAGS) -o $@ -c $<

clean: 
	\rm -f *.o *.x
