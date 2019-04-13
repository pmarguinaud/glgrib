

glgrib.x: load.o glgrib.o glgrib_png.o glgrib_fb.o glgrib_render.o glgrib_x11.o shader.o
	g++  -std=c++11  -g -o glgrib.x load.o glgrib.o  glgrib_png.o glgrib_fb.o  glgrib_render.o  glgrib_x11.o  shader.o -leccodes -lGLEW -lGL -lEGL -lglfw -lpng

load.o: load.c
	cc -c -g load.c

glgrib.o: glgrib.cc
	c++ -std=c++11 -g -c glgrib.cc

glgrib_png.o: glgrib_png.cc
	c++ -std=c++11 -g -c glgrib_png.cc

glgrib_fb.o: glgrib_fb.cc
	c++ -std=c++11 -g -c glgrib_fb.cc

glgrib_render.o: glgrib_render.cc
	c++ -std=c++11 -g -c glgrib_render.cc

glgrib_x11.o: glgrib_x11.cc
	c++ -std=c++11 -g -c glgrib_x11.cc

shader.o: shader.cc
	c++ -std=c++11 -g -c shader.cc



