

CXXFLAGS=-std=c++11 -g -fopenmp
LDFLAGS=-fopenmp -leccodes -lGLEW -lGL -lglfw -lpng -lreadline -ltinfo -lssl -lcrypto -lpthread -lsqlite3
GDB=gdb -ex='set confirm on' -ex=run -ex=quit --args

CXXFLAGS=-fopenmp -std=c++11 -g -I$(HOME)/3d/usr/include -L$(HOME)/3d/usr/lib64 -Wl,-rpath,$(HOME)/3d/usr/lib64 -leccodes -lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto
CXXFLAGS += -I$(HOME)/install/eccodes--2.13.0_FIXOMMCODES/include -L$(HOME)/install/eccodes--2.13.0_FIXOMMCODES/lib -Wl,-rpath,$(HOME)/install/eccodes--2.13.0_FIXOMMCODES/lib

glgrib.x: glgrib_field_vector.o glgrib_field_scalar.o glgrib_geometry_lambert.o glgrib_projection.o glgrib_colorbar.o glgrib_font.o glgrib_string.o glgrib_geometry_latlon.o glgrib_window_offscreen.o glgrib_geometry.o glgrib_geometry_gaussian.o glgrib_window.o glgrib_options.o glgrib_shell.o glgrib_bmp.o glgrib_landscape.o glgrib_palette.o glgrib_field.o glgrib_load.o glgrib_polygon.o glgrib_program.o glgrib_view.o glgrib_world.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_shader.o
	g++  $(CXXFLAGS) -o glgrib.x glgrib_field_vector.o glgrib_field_scalar.o glgrib_geometry_lambert.o glgrib_projection.o glgrib_colorbar.o glgrib_font.o glgrib_string.o glgrib_geometry_latlon.o glgrib_window_offscreen.o glgrib_geometry.o glgrib_geometry_gaussian.o glgrib_window.o glgrib_options.o glgrib_shell.o glgrib_bmp.o glgrib_landscape.o glgrib_palette.o glgrib_field.o glgrib_load.o glgrib_polygon.o glgrib_view.o glgrib_program.o glgrib_world.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_scene.o glgrib_coastlines.o glgrib_grid.o glgrib_shader.o $(LDFLAGS)

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

test_colorbar: glgrib.x
	$(GDB) ./glgrib.x --landscape.geometry t1198c2.2/Z.grb --field[0].path t1198c2.2/N.grb  --field[0].scale 1.03  --grid.resolution 0 --coastlines.path "" \
		--colorbar.on --colorbar.font.r 0 --colorbar.font.g 255 --colorbar.font.b 0 --window.width 1000 \
		--font.r 0 --font.g 255 --font.b 0

test_bw: glgrib.x
	$(GDB) ./glgrib.x --landscape.geometry t1198c2.2/Z.grb --field[0].path t1198c2.2/N.grb  --field[0].scale 1.03  --grid.resolution 0 --coastlines.path ""

test_bw_debug: glgrib.x
	$(GDB) ./glgrib.x --landscape.geometry t1198c2.2/Z.grb --field[0].path t1198c2.2/N.grb  --field[0].scale 1.03  --grid.resolution 0 --coastlines.path "" --window.debug

test_3l_t1198: glgrib.x
	$(GDB) ./glgrib.x --landscape.geometry t1198c2.2/Z.grb --grid.resolution 0 --coastlines.path ""  \
               --field[0].path t1198c2.2/SURFNEBUL.BASSE.grb --field[1].path t1198c2.2/SURFNEBUL.MOYENN.grb --field[2].path t1198c2.2/SURFNEBUL.HAUTE.grb \
               --field[0].scale                         1.03 --field[1].scale                          1.04 --field[2].scale                         1.05 \
               --field[0].palette                      cloud --field[1].palette                       cloud --field[2].palette                      cloud 

test_3l_t1798: glgrib.x
	$(GDB) ./glgrib.x --landscape.geometry t1798/Z.grb --grid.resolution 0 --coastlines.path ""  \
                --field[0].path t1798/SURFNEBUL.BASSE.grb --field[1].path t1798/SURFNEBUL.MOYENN.grb --field[2].path t1798/SURFNEBUL.HAUTE.grb \
                --field[0].scale                     1.03 --field[1].scale                      1.04 --field[2].scale                     1.05 \
                --field[0].palette                  cloud --field[1].palette                   cloud --field[2].palette                  cloud


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

test_landscape_eurat01: ./glgrib.x
	$(GDB) ./glgrib.x --landscape.geometry  eurat01/lfpw_0_0_0_pl_1000_t.grib2 --landscape.orography 0

test_glob01: ./glgrib.x
	$(GDB) ./glgrib.x --landscape.path "" --field[0].path glob01/lfpw_0_0_0_sfc_0_t.grib2 --field[0].scale 1.00  --field[0].palette cold_hot_temp


test_small: ./glgrib.x
	$(GDB) ./glgrib.x --landscape.geometry  t49/Z.grb  --field[0].scale 1.01  --field[0].path t49/SFX.CLAY.grb

test_shell: ./glgrib.x
	./glgrib.x --shell --landscape.geometry  t49/Z.grb  --field[0].scale 1.01  --field[0].path t49/SFX.CLAY.grb

test_novalue: ./glgrib.x
	$(GDB) ./glgrib.x --landscape.geometry t1798/Z.grb --grid.resolution 0 --coastlines.path ""  \
		--field[0].path t1798/SURFNEBUL.BASSE.grb --field[0].scale 1.03 --field[0].palette cloud_auto --field[0].no_value_pointer

test_t8000_noorog: ./glgrib.x
	$(GDB) ./glgrib.x  --window.width 2000  --window.height 2000 --landscape.geometry t479/Z.grb \
		--landscape.orography 0 --grid.resolution 0 --coastlines.path ""   \
		--window.offscreen    --field[0].path t8000/SURFNEBUL.TOTALE.grb --field[0].scale 1.03 --field[0].palette cloud_auto  --field[0].no_value_pointer  \
		--scene.light.on --scene.light.lon -25 --scene.light.lat 30.

test_missingvalue: ./glgrib.x
	$(GDB) ./glgrib.x --landscape.path "" --field[0].path t49/SFX.CLAY.grb

test_aro: ./glgrib.x
	$(GDB) ./glgrib.x --field[0].path ./aro2.5/SURFIND.TERREMER.grb --field[0].palette cold_hot --field[0].scale 1.00 --landscape.path "" --camera.lat 46.2 --camera.lon 2.0 --camera.fov 5

test_guyane: ./glgrib.x
	$(GDB) ./glgrib.x --field\[0\].path ./aro_guyane/SURFTEMPERATURE.grb --field\[0\].palette cold_hot_temp --field\[0\].scale 1.01 --camera.lat 5 --camera.lon -51 --camera.fov 3

test_vector: ./glgrib.x
	$(GDB) ./glgrib.x --field[0].vector --field\[0\].path ./aro2.5/S090WIND.U.PHYS.grb  ./aro2.5/S090WIND.V.PHYS.grb --field\[0\].scale 1.00 --landscape.path '' --camera.lat 46.2 --camera.lon 2.0 --camera.fov 5

test_small_aro: ./glgrib.x
	$(GDB) ./glgrib.x --field[0].vector --field\[0\].path ./aro_small/S041WIND.U.PHYS.grb ./aro_small/S041WIND.V.PHYS.grb  --field\[0\].scale 1.00 --landscape.path '' --camera.lon 26.64 --camera.lat 67.36 --camera.fov 0.5

test_all: test_colorbar test_bw test_bw_debug test_3l_t1198 test_3l_t1798 test_offscreen test_eurat01 test_landscape_eurat01 test_glob01 test_small test_shell test_novalue test_t8000_noorog test_missingvalue test_aro test_guyane
