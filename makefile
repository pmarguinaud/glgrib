

LDFLAGS=-fopenmp -leccodes -lGLEW -lGL -lglfw -lpng -lreadline -ltinfo -lssl -lcrypto -lpthread -lsqlite3
RUNTEST=./runtest.pl $@

CXXFLAGS=-O2 -fopenmp -std=c++11 -g -I$(HOME)/3d/usr/include -L$(HOME)/3d/usr/lib64 -Wl,-rpath,$(HOME)/3d/usr/lib64 -leccodes -lGLEW -lGL -lglfw -lpng -lreadline -lncurses -ltinfo -lssl -lcrypto
CXXFLAGS += -I$(HOME)/install/eccodes--2.13.0_FIXOMMCODES/include -L$(HOME)/install/eccodes--2.13.0_FIXOMMCODES/lib -Wl,-rpath,$(HOME)/install/eccodes--2.13.0_FIXOMMCODES/lib

all: glgrib.x

glgrib.x: glgrib_mapscale.o glgrib_rivers.o glgrib_border.o glgrib_lines.o glgrib_gshhg.o glgrib_image.o glgrib_resolve.o glgrib_field_float_buffer.o glgrib_field_contour.o glgrib_field_vector.o glgrib_field_scalar.o glgrib_geometry_lambert.o glgrib_projection.o glgrib_colorbar.o glgrib_font.o glgrib_string.o glgrib_geometry_latlon.o glgrib_window_offscreen.o glgrib_geometry.o glgrib_geometry_gaussian.o glgrib_window.o glgrib_options.o glgrib_shell.o glgrib_bitmap.o glgrib_landscape.o glgrib_palette.o glgrib_field.o glgrib_loader.o glgrib_polygon.o glgrib_program.o glgrib_view.o glgrib_world.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_scene.o glgrib_coast.o glgrib_grid.o glgrib_shader.o 
	g++  $(CXXFLAGS) -o glgrib.x glgrib_mapscale.o glgrib_rivers.o glgrib_border.o glgrib_lines.o glgrib_gshhg.o glgrib_image.o glgrib_resolve.o glgrib_field_float_buffer.o glgrib_field_contour.o glgrib_field_vector.o glgrib_field_scalar.o glgrib_geometry_lambert.o glgrib_projection.o glgrib_colorbar.o glgrib_font.o glgrib_string.o glgrib_geometry_latlon.o glgrib_window_offscreen.o glgrib_geometry.o glgrib_geometry_gaussian.o glgrib_window.o glgrib_options.o glgrib_shell.o glgrib_bitmap.o glgrib_landscape.o glgrib_palette.o glgrib_field.o glgrib_loader.o glgrib_polygon.o glgrib_view.o glgrib_program.o glgrib_world.o glgrib.o glgrib_opengl.o glgrib_png.o glgrib_scene.o glgrib_coast.o glgrib_grid.o glgrib_shader.o $(LDFLAGS) -Llfi -llfi -lcurl

%.o: %.cc
	g++ $(CXXFLAGS) -o $@ -c $<

clean: 
	\rm -f *.o *.x

glwhat.x: glwhat.cc
	g++ $(CXXFLAGS) -g -o glwhat.x glwhat.cc $(LDFLAGS)

test_colorbar: glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb  --field[0].scale 1.03   \
		--colorbar.on --colorbar.font.color.foreground green --window.width 1000 

test_bw: glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb  --field[0].scale 1.03  

test_bw_debug: glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb  --field[0].scale 1.03 --window.debug.on

test_3l_t1198: glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb \
               --field[0].path testdata/t1198c2.2/SURFNEBUL.BASSE.grb --field[1].path testdata/t1198c2.2/SURFNEBUL.MOYENN.grb --field[2].path testdata/t1198c2.2/SURFNEBUL.HAUTE.grb \
               --field[0].scale                         1.03 --field[1].scale                          1.04 --field[2].scale                         1.05 \
               --field[0].palette.name                 cloud --field[1].palette.name                  cloud --field[2].palette.name                 cloud 

test_3l_t1798: glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1798/Z.grb \
                --field[0].path testdata/t1798/SURFNEBUL.BASSE.grb --field[1].path testdata/t1798/SURFNEBUL.MOYENN.grb --field[2].path testdata/t1798/SURFNEBUL.HAUTE.grb \
                --field[0].scale                     1.03 --field[1].scale                      1.04 --field[2].scale                     1.05 \
                --field[0].palette.name             cloud --field[1].palette.name              cloud --field[2].palette.name             cloud


test_offscreen: glgrib.x
	\rm -f snapshot*.png
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --window.offscreen.on  \
		--window.offscreen.frames 10 --scene.light.rotate.on --scene.light.on     \
		--field[0].path testdata/t1198c2.2/SURFNEBUL.BASSE.grb testdata/t1198c2.2/SURFNEBUL.MOYENN.grb  testdata/t1198c2.2/SURFNEBUL.HAUTE.grb \
		--field[0].scale                         1.03                           1.03                           1.03 \
		--field[0].palette.name            cloud_auto                     cloud_auto                     cloud_auto 

test_eurat01: glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb \
               --field[0].path testdata/t1198c2.2/N.grb  --field[1].path testdata/eurat01/lfpw_0_0_0_pl_1000_t.grib2 \
               --field[0].scale           1.02  --field[1].scale                              1.03 \
               --field[0].palette.name   cloud_auto  --field[1].palette.name                   cold_hot 

test_landscape_eurat01: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry  testdata/eurat01/lfpw_0_0_0_pl_1000_t.grib2 --landscape.orography 0 --coast.on --grid.on

test_glob01: ./glgrib.x
	$(RUNTEST) ./glgrib.x --field[0].path testdata/glob01/lfpw_0_0_0_sfc_0_t.grib2 --field[0].scale 1.00  --field[0].palette.name cold_hot_temp --coast.on --grid.on


test_small: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry  testdata/t49/Z.grb  --field[0].scale 1.01  --field[0].path testdata/t49/SFX.CLAY.grb --coast.on --grid.on

test_novalue: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1798/Z.grb \
		--field[0].path testdata/t1798/SURFNEBUL.BASSE.grb --field[0].scale 1.03 --field[0].palette.name cloud_auto --field[0].no_value_pointer.on

test_missingvalue: ./glgrib.x
	$(RUNTEST) ./glgrib.x --field[0].path testdata/t49/SFX.CLAY.grb --coast.on --grid.on

test_aro: ./glgrib.x
	$(RUNTEST) ./glgrib.x --field[0].path testdata/aro2.5/SURFIND.TERREMER.grb --field[0].palette.name cold_hot --field[0].scale 1.00 --view.lat 46.2 --view.lon 2.0 --view.fov 5 --coast.on --grid.on

test_guyane: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --field\[0\].path testdata/aro_guyane/SURFTEMPERATURE.grb --field\[0\].palette.name cold_hot_temp --field\[0\].scale 1.01 --view.lat 5 --view.lon -51 --view.fov 3 --coast.on --grid.on

test_vector: ./glgrib.x
	$(RUNTEST) ./glgrib.x --field[0].vector.on --field\[0\].path testdata/aro2.5/S090WIND.U.PHYS.grb  testdata/aro2.5/S090WIND.V.PHYS.grb --field\[0\].scale 1.00 --field\[0\].vector.color green --view.lat 46.2 --view.lon 2.0 --view.fov 5  --coast.on --grid.on

test_small_aro: ./glgrib.x
	$(RUNTEST) ./glgrib.x --field[0].vector.on --field\[0\].path testdata/aro_small/S041WIND.U.PHYS.grb testdata/aro_small/S041WIND.V.PHYS.grb  --field\[0\].scale 1.00 --view.lon 26.64 --view.lat 67.36 --view.fov 0.5 --coast.on --grid.on

test_wind_arp: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t31c2.4/Z.grb --field[0].vector.on --field\[0\].path testdata/t31c2.4/S015WIND.U.PHYS.grb testdata/t31c2.4/S015WIND.V.PHYS.grb   --field[0].scale 1.01 --coast.on --grid.on

test_vector_glob25: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 --landscape.orography 0  --field[0].vector.on \
		--field\[0\].path testdata/arpt1798_wind/glob25_+1.grb testdata/arpt1798_wind/glob25_+1.grb  --field[0].scale 1.01 --coast.on --grid.on

test_wind_glob25: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 --landscape.orography 0  --field[0].vector.on \
		--field\[0\].path testdata/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 testdata/arpt1798_wind/lfpw_0_2_3_sfc_20_v.grib2  --field[0].scale 1.01 --coast.on --grid.on

test_wind_t1798: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/arpt1798_wind/S105WIND.U.PHYS.grb --landscape.orography 0  --field[0].vector.on \
		--field\[0\].path testdata/arpt1798_wind/S105WIND.U.PHYS.grb testdata/arpt1798_wind/S105WIND.V.PHYS.grb  --field[0].scale 1.01 --coast.on --grid.on

test_vector_t1798: ./glgrib.x
	$(RUNTEST) ./glgrib.x  --landscape.on --landscape.geometry testdata/arpt1798_wind/+1.grb --landscape.orography 0 --field[0].vector.on \
		--field\[0\].path testdata/arpt1798_wind/+1.grb testdata/arpt1798_wind/+1.grb  --field[0].scale 1.01 --coast.on --grid.on

test_contour1: ./glgrib.x
	$(RUNTEST) ./glgrib.x --window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp  --field\[0\].path testdata/contour/t0049.grb --field\[0\].scale 1.03 --field\[0\].contour.on

test_contour2: ./glgrib.x
	$(RUNTEST) ./glgrib.x --window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp  --field\[0\].path testdata/contour/t0479.grb --field\[0\].scale 1.03 --field\[0\].contour.on

test_contour3: ./glgrib.x
	$(RUNTEST) ./glgrib.x --window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp  --field\[0\].path testdata/contour/t1798.grb --field\[0\].scale 1.03 --field\[0\].contour.on

test_contour_stretched: ./glgrib.x
	$(RUNTEST) ./glgrib.x --window.width 1024 --window.height 1024 --field\[0\].path testdata/t1198c2.2/Z.grb --field\[0\].scale 1.03 --field\[0\].contour.on

test_contour_latlon1: ./glgrib.x
	$(RUNTEST) ./glgrib.x --window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp  --field\[0\].path testdata/contour/latlon9x6.grb --field\[0\].scale 1.03 --field\[0\].contour.on

test_contour_latlon2: ./glgrib.x
	$(RUNTEST) ./glgrib.x --window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp  --field\[0\].path testdata/contour/latlon18x13.grb --field\[0\].scale 1.03 --field\[0\].contour.on

test_contour_latlon3: ./glgrib.x
	$(RUNTEST) ./glgrib.x --window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp  --field\[0\].path testdata/eurat01/lfpw_0_0_0_pl_1000_t.grib2 --field\[0\].scale 1.03 --field\[0\].contour.on

test_contour_latlon4: ./glgrib.x
	$(RUNTEST) ./glgrib.x --window.width 1024 --window.height 1024 --field\[0\].path testdata/glob025/lfpw_0_0_0_pl_1000_t.grib2 --field\[1\].path testdata/glob025/lfpw_0_0_0_pl_1000_t.grib2 --field\[1\].palette.name cold_hot --field\[1\].contour.on

test_contour_aro1: ./glgrib.x
	$(RUNTEST) ./glgrib.x \
               --field\[0\].path testdata/aro_small/S041WIND.U.PHYS.grb --field\[0\].scale 1.00 --field[0].palette.name cold_hot \
               --field\[1\].path testdata/aro_small/S041WIND.U.PHYS.grb --field\[1\].scale 1.03 --field[1].contour.on       \
               --view.lon 26.64 --view.lat 67.36 --view.fov 0.5 --coast.on --grid.on

test_contour_aro2: ./glgrib.x
	$(RUNTEST) ./glgrib.x \
               --field\[0\].path testdata/aro2.5/S090WIND.U.PHYS.grb --field\[0\].scale 1.00 --field[0].palette.name cold_hot \
               --field\[1\].path testdata/aro2.5/S090WIND.U.PHYS.grb --field\[1\].scale 1.03 --field[1].contour.on       \
               --view.lat 46.2 --view.lon 2.0 --view.fov 5 --coast.on --grid.on

test_fill: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --field\[0\].path testdata/t1198c2.2/N.grb --field\[0\].scale 1.03 --field[0].palette.name RGBW   --colorbar.on --window.width 1200

test_dashed1: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.orography 0 --landscape.wireframe.on --landscape.geometry testdata/contour/t0479.grb --window.width 1024 --window.height 1024 \
		--field\[0\].path testdata/contour/t0479.grb --field\[0\].scale 1.00 --field\[0\].contour.on --field[0].contour.lengths 100  --field[0].contour.patterns X- --view.fov 5

test_dashed2: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.orography 0 --landscape.wireframe.on --landscape.geometry testdata/contour/t0479.grb --window.width 1024 --window.height 1024 \
		--field\[0\].path testdata/contour/t0479.grb --field\[0\].scale 1.01 --field\[0\].contour.on --field\[0\].contour.lengths 100 --field\[0\].contour.patterns XXXXXXXXX-X- --view.fov 5  --field\[0\].contour.widths 5

test_dashed3: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.orography 0 --landscape.wireframe.on --landscape.geometry testdata/contour/t0479.grb --window.width 1024 --window.height 1024 \
		--field\[0\].path testdata/contour/t0479.grb --field\[0\].scale 1.01 --field\[0\].contour.on --field\[0\].contour.lengths 100 --field\[0\].contour.patterns XXXXXXXXX-X- --view.fov 5  --field\[0\].contour.widths 5 --field\[0\].contour.colors red green blue

test_travelling: ./glgrib.x
	$(RUNTEST) ./glgrib.x --scene.travelling.on --scene.travelling.pos1.lon 0 --scene.travelling.pos1.lat 0 --scene.travelling.pos1.fov 30 \
		--scene.travelling.pos2.lon 90 --scene.travelling.pos2.lat 45 --scene.travelling.pos2.fov 10 --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field\[0\].path testdata/t1198c2.2/N.grb --field\[0\].scale 1.03 

test_strxyz: ./glgrib.x
	$(RUNTEST) ./glgrib.x --field\[0\].path testdata/t1198c2.2/N.grb --field\[0\].scale 0.99 \
		--colorbar.on --colorbar.font.color.foreground green --window.width 1000 --scene.test_strxyz.font.color.foreground green \
		--scene.projection XYZ --scene.test_strxyz.on

test_interpolation: ./glgrib.x
	$(RUNTEST) ./glgrib.x --field\[0\].path testdata/interp/01.grb testdata/interp/36.grb  --field\[0\].scale 1.03 --field\[0\].palette.name cold_hot_temp  \
		--view.lon 2 --view.lat 46.7 --view.fov 5 --scene.interpolation.on --scene.interpolation.frames 200 --window.width 1000 --window.height 1000 \
                --scene.date.font.scale 0.03 --scene.date.font.color.foreground red --scene.date.on --coast.on --grid.on

test_background: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field\[0\].path testdata/t1198c2.2/N.grb --field\[0\].scale 1.03 \
		--scene.date.on  --scene.date.font.scale 0.03 --scene.date.font.color.foreground red --scene.date.font.color.background white

test_text: ./glgrib.x
	$(RUNTEST) ./glgrib.x  --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field\[0\].path testdata/t1198c2.2/N.grb --field\[0\].scale 1.03 \
		--scene.text.on --scene.text.s "coucou" --scene.text.x 0.0 --scene.text.y 1.0 --scene.text.a NW  --scene.text.font.scale 0.03 --scene.text.font.color.foreground black --scene.text.font.color.background white

test_fa: ./glgrib.x
	$(RUNTEST) ./glgrib.x --field\[0\].path testdata/fa/PGD.t479.fa%SFX.FRAC_SEA  --field\[0\].scale 1.03 

test_select_grib: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field\[0\].path testdata/t1198c2.2/N.grb%'parameterCategory=6,year=2019' --field\[0\].scale 1.03 

test_scalar_diff: ./glgrib.x
	$(RUNTEST) ./glgrib.x --field[0].diff.on --field\[0\].path testdata/interp/01.grb testdata/interp/36.grb --field\[0\].scale 1.01  --view.lon 2 --view.lat 46.7 --view.fov 5 

test_contour_diff: ./glgrib.x
	$(RUNTEST) ./glgrib.x --field[0].diff.on --field[0].contour.on --field\[0\].path testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0000.grib2 testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0102.grib2 \
		--field\[0\].scale 1.01  

test_mercator: ./glgrib.x
	$(RUNTEST) ./glgrib.x --field[0].diff.on --field[0].contour.on --field\[0\].path testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0000.grib2 testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0102.grib2 \
		--field\[0\].scale 1.01  --view.projection MERCATOR --landscape.on

test_image: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field\[0\].path testdata/t1198c2.2/N.grb --field\[0\].scale 1.03 \
		--scene.image.on --scene.image.path testdata/image/B.bmp --scene.image.x0 0. --scene.image.y0 0.  --scene.image.x1 0.1 --scene.image.y1 0.1

test_shell: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field\[0\].path testdata/t1198c2.2/N.grb --field\[0\].scale 1.03 --shell.on 

test_mapscale: ./glgrib.x
	$(RUNTEST) ./glgrib.x --landscape.on --mapscale.on

test_all: test_colorbar test_bw test_bw_debug test_3l_t1198 test_3l_t1798 test_offscreen test_eurat01 test_landscape_eurat01 test_glob01 test_small test_novalue test_missingvalue test_aro test_guyane test_vector test_small_aro test_wind_arp test_vector_glob25 test_wind_glob25 test_wind_t1798 test_vector_t1798 test_contour1 test_contour2 test_contour3 test_contour_stretched test_contour_latlon1 test_contour_latlon2 test_contour_latlon3 test_contour_latlon4 test_contour_aro1 test_contour_aro2 test_fill test_dashed1 test_dashed2 test_dashed3 test_travelling test_strxyz test_interpolation test_background test_text test_fa test_select_grib test_scalar_diff test_contour_diff test_image test_shell test_largebmp
