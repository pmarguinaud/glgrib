

CXXFLAGS=-std=c++11 -g -fopenmp
LDFLAGS=-fopenmp -leccodes -lGLEW -lGL -lglfw -lpng -lreadline -ltinfo -lssl -lcrypto
GDB=./gdbwrap
GDB=

CXXFLAGS=-fopenmp -std=c++11 -g -I$(HOME)/install/eccodes-2.12.0/include -I$(HOME)/3d/usr/include -L$(HOME)/3d/usr/lib64 -Wl,-rpath,$(HOME)/3d/usr/lib64 -L$(HOME)/install/eccodes-2.12.0/lib -Wl,-rpath,$(HOME)/install/eccodes-2.12.0/lib -leccodes -lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto

glgrib.x: glgrib_geometry_latlon.o glgrib_window_offscreen.o glgrib_geometry.o glgrib_geometry_gaussian.o glgrib_window.o glgrib_options.o glgrib_shell.o glgrib_bmp.o glgrib_landscape.o glgrib_palette.o glgrib_field.o glgrib_load.o glgrib_polygon.o glgrib_program.o glgrib_view.o glgrib_polyhedron.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_world.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_shader.o
	g++  $(CXXFLAGS) -o glgrib.x glgrib_geometry_latlon.o glgrib_window_offscreen.o glgrib_geometry.o glgrib_geometry_gaussian.o glgrib_window.o glgrib_options.o glgrib_shell.o glgrib_bmp.o glgrib_landscape.o glgrib_palette.o glgrib_field.o glgrib_load.o glgrib_polygon.o glgrib_view.o glgrib_program.o glgrib_polyhedron.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_world.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_shader.o $(LDFLAGS)

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
	$(GDB) ./glgrib.x --landscape.geometry t1198c2.2/Z.grb --field[0].path t1198c2.2/N.grb  --field[0].scale 1.03  --grid.resolution 0 --coastlines.path ""

test_3l_t1198: glgrib.x
	$(GDB) ./glgrib.x --landscape.geometry t1198c2.2/Z.grb --grid.resolution 0 --coastlines.path ""  \
               --field[0].path t1198c2.2/SURFNEBUL.BASSE.grb --field[1].path t1198c2.2/SURFNEBUL.MOYENN.grb --field[2].path t1198c2.2/SURFNEBUL.HAUTE.grb \
               --field[0].scale                         1.03 --field[1].scale                          1.04 --field[2].scale                         1.05 \
               --field[0].palette                 cloud_auto --field[1].palette                  cloud_auto --field[2].palette                 cloud_auto 

test_3l_t1798: glgrib.x
	$(GDB) ./glgrib.x --landscape.geometry t1798/Z.grb --grid.resolution 0 --coastlines.path ""  \
                --field[0].path t1798/SURFNEBUL.BASSE.grb --field[1].path t1798/SURFNEBUL.MOYENN.grb --field[2].path t1798/SURFNEBUL.HAUTE.grb \
                --field[0].scale                     1.03 --field[1].scale                      1.04 --field[2].scale                     1.05 \
                --field[0].palette             cloud_auto --field[1].palette              cloud_auto --field[2].palette             cloud_auto


test_offscreen: glgrib.x
	\rm -f snapshot*.png
	$(GDB) ./glgrib.x --landscape.geometry t1198c2.2/Z.grb --grid.resolution 0 --coastlines.path ""  --window.offscreen  \
		--window.offscreen_frames 10 --scene.movie --scene.movie-wait -1  --scene.light.rotate --scene.light.on     \
		--field[0].path t1198c2.2/SURFNEBUL.BASSE.grb t1198c2.2/SURFNEBUL.MOYENN.grb  t1198c2.2/SURFNEBUL.HAUTE.grb \
		--field[0].scale                         1.03                           1.03                           1.03 \
		--field[0].palette                 cloud_auto                     cloud_auto                     cloud_auto 

test_eurat01: glgrib.x
	$(GDB) ./glgrib.x --landscape.geometry t1198c2.2/Z.grb --grid.resolution 0 --coastlines.path "" \
               --field[0].path t1198c2.2/N.grb  --field[1].path eurat01/lfpw_0_0_0_pl_1000_t.grib2 \
               --field[0].scale           1.02  --field[1].scale                              1.03 \
               --field[0].palette   cloud_auto  --field[1].palette                        cold_hot 

test_landscape_eurat01:
	$(GDB) ./glgrib.x --landscape.geometry  eurat01/lfpw_0_0_0_pl_1000_t.grib2 --landscape.orography 0

test_glob01:
	$(GDB) ./glgrib.x --landscape.path "" --field[0].path glob01/lfpw_0_0_0_sfc_0_t.grib2 --field[0].scale 1.00  --field[0].palette cold_hot_temp


test_small:
	$(GDB) ./glgrib.x --landscape.geometry  t49/Z.grb  --field[0].scale 1.01  --field[0].path t49/SFX.CLAY.grb

test_shell:
	./glgrib.x --shell --landscape.geometry  t49/Z.grb  --field[0].scale 1.01  --field[0].path t49/SFX.CLAY.grb

test_novalue:
	$(GDB) ./glgrib.x --landscape.geometry t1798/Z.grb --grid.resolution 0 --coastlines.path ""  \
		--field[0].path t1798/SURFNEBUL.BASSE.grb --field[0].scale 1.03 --field[0].palette cloud_auto --field[0].no_value_pointer

test_t8000_noorog:
	$(GDB) ./glgrib.x  --window.width 2000  --window.height 2000 --landscape.geometry t479/Z.grb \
		--landscape.orography 0 --grid.resolution 0 --coastlines.path ""   \
		--window.offscreen    --field[0].path t8000/SURFNEBUL.TOTALE.grb --field[0].scale 1.03 --field[0].palette cloud_auto  --field[0].no_value_pointer  \
		--scene.light.on --scene.light.lon -25 --scene.light.lat 30.

test_missingvalue:
	$(GDB) ./glgrib.x --landscape.path "" --field[0].path t49/SFX.CLAY.grb
