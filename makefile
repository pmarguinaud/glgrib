

FLAGS=-std=c++11 -g 

glgrib.x: glgrib_load.o glgrib.o glgrib_png.o glgrib_fb.o glgrib_render.o glgrib_x11.o shader.o
	g++  $(FLAGS) -o glgrib.x glgrib_load.o glgrib.o  glgrib_png.o glgrib_fb.o  glgrib_render.o  glgrib_x11.o  shader.o -leccodes -lGLEW -lGL -lEGL -lglfw -lpng

glgrib_load.o: glgrib_load.cc
	c++ $(FLAGS) -c glgrib_load.cc

glgrib.o: glgrib.cc
	c++ $(FLAGS) -c glgrib.cc

glgrib_png.o: glgrib_png.cc
	c++ $(FLAGS) -c glgrib_png.cc

glgrib_fb.o: glgrib_fb.cc
	c++ $(FLAGS) -c glgrib_fb.cc

glgrib_render.o: glgrib_render.cc
	c++ $(FLAGS) -c glgrib_render.cc

glgrib_x11.o: glgrib_x11.cc
	c++ $(FLAGS) -c glgrib_x11.cc

shader.o: shader.cc
	c++ $(FLAGS) -c shader.cc


clean: 
	\rm *.o *.x
