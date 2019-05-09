

CXXFLAGS=-std=c++11 -g -fopenmp
LDFLAGS=-fopenmp -leccodes -lGLEW -lGL -lglfw -lpng -lreadline -ltinfo -lssl -lcrypto

CXXFLAGS=-fopenmp -std=c++11 -g -I$(HOME)/install/eccodes-2.12.0/include -I$(HOME)/3d/usr/include -L$(HOME)/3d/usr/lib64 -Wl,-rpath,$(HOME)/3d/usr/lib64 -L$(HOME)/install/eccodes-2.12.0/lib -Wl,-rpath,$(HOME)/install/eccodes-2.12.0/lib -leccodes -lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto

glgrib.x: glgrib_window_offscreen.o glgrib_geometry.o glgrib_geometry_gaussian.o glgrib_window.o glgrib_options.o glgrib_shell.o glgrib_bmp.o glgrib_landscape.o glgrib_palette.o glgrib_field.o glgrib_load.o glgrib_polygon.o glgrib_program.o glgrib_view.o glgrib_polyhedron.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_world.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_x11.o glgrib_shader.o
	g++  $(CXXFLAGS) -o glgrib.x glgrib_window_offscreen.o glgrib_geometry.o glgrib_geometry_gaussian.o glgrib_window.o glgrib_options.o glgrib_shell.o glgrib_bmp.o glgrib_landscape.o glgrib_palette.o glgrib_field.o glgrib_load.o glgrib_polygon.o glgrib_view.o glgrib_program.o glgrib_polyhedron.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_world.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_x11.o  glgrib_shader.o $(LDFLAGS)

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

test_bw: glgrib.x
	./glgrib.x --landscape.geometry t1198c2.2/Z.grb --field.list t1198c2.2/N.grb  --field.scale 1.03  --grid.resolution 0 --coastlines.path ""

test_3l: glgrib.x
	./glgrib.x --landscape.geometry t1198c2.2/Z.grb --field.list t1198c2.2/SURFNEBUL.BASSE.grb t1198c2.2/SURFNEBUL.MOYENN.grb t1198c2.2/SURFNEBUL.HAUTE.grb \
		--field.scale 1.03 1.04 1.05  --grid.resolution 0 --coastlines.path ""  --field.palette cloud_auto cloud_auto cloud_auto

test_movie: glgrib.x
	./glgrib.x --landscape.geometry t1198c2.2/Z.grb --field.list t1198c2.2/SURFNEBUL.BASSE.grb t1198c2.2/SURFNEBUL.MOYENN.grb t1198c2.2/SURFNEBUL.HAUTE.grb \
		--field.scale 1.03 1.03 1.03  --grid.resolution 0 --coastlines.path ""  --field.palette cloud_auto cloud_auto cloud_auto --scene.movie

test_offscreen: glgrib.x
	./glgrib.x --landscape.geometry t1198c2.2/Z.grb --field.list t1198c2.2/SURFNEBUL.BASSE.grb t1198c2.2/SURFNEBUL.MOYENN.grb t1198c2.2/SURFNEBUL.HAUTE.grb \
		--field.scale 1.03 1.04 1.05  --grid.resolution 0 --coastlines.path ""  --field.palette cloud_auto cloud_auto cloud_auto --window.offscreen \
		--scene.rotate-light --scene.light --window.offscreen_frames 10


