#!/usr/bin/perl -w
#
use strict;
use File::Path;
use Data::Dumper;
use Text::ParseWords;
use FileHandle;


my (%test, @test);

sub help
{
  my @t = @test;
  while (my ($desc, $name, $opts) = splice (@t, 0, 3))
    {
      print "* $name\n";
      print "   $desc\n";
      my @o = @{ $test{$name}[1] };
      my @l = ('');
      while (my $o = shift (@o))
        {
          push @l, '' if (length ($l[-1]) + length ($o) > 70);
          $l[-1] .= "$o ";
        }
      for (@l)
        {
          print "    $_\n";
        }
      print "\n";
    }
  die ("\n");
}

sub readme
{
  my @t = @test;
  print "
Display GRIB2 fields with OpenGL. Raster, contour, vector, colorbar, mapscale, coastlines, borders. Lat/lon, lambert, gaussian grid.
  ";
  while (my ($desc, $name, $opts) = splice (@t, 0, 3))
    {
      next unless ($opts =~ m/^--/o);
      print "## $desc -- $name\n";
      print "![](test.ref/$name/TEST_0000.png)\n";
      print "\n";

      my @o = @{ $test{$name}[1] };

      my @l = ('');
      while (defined (my $o = shift (@o)))
        {
          push @l, '' if (length ($l[-1]) + length ($o) > 70);
          $l[-1] .= "$o ";
        }

      print "```\n";
      for (@l)
        {
          print "    $_\n";
        }
      print "```\n";

    }
  die ("\n");
}


@test =
(
  "Clouds, 3 layers, t1198c2.2                                      " ,  t1198_3l            => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/SURFNEBUL.BASSE.grb --field[1].path testdata/t1198c2.2/SURFNEBUL.MOYENN.grb --field[2].path testdata/t1198c2.2/SURFNEBUL.HAUTE.grb --field[0].scale 1.03 --field[1].scale 1.04 --field[2].scale 1.05 --field[0].palette.name cloud --field[1].palette.name cloud --field[2].palette.name cloud',
  "Clouds, 3 layers, t1798c2.2                                      " ,  t1798_3l            => '--landscape.on --landscape.geometry testdata/t1798/Z.grb --field[0].path testdata/t1798/SURFNEBUL.BASSE.grb --field[1].path testdata/t1798/SURFNEBUL.MOYENN.grb --field[2].path testdata/t1798/SURFNEBUL.HAUTE.grb --field[0].scale 1.03 --field[1].scale 1.04 --field[2].scale 1.05 --field[0].palette.name cloud --field[1].palette.name cloud --field[2].palette.name cloud',
  "AROME, Lambert geometry                                          " ,  aro                 => '--field[0].path testdata/aro2.5/SURFIND.TERREMER.grb --field[0].palette.name cold_hot --field[0].scale 1.00 --view.lat 46.2 --view.lon 2.0 --view.fov 5 --coast.on --grid.on',
  "Font background color                                            " ,  background          => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[0].scale 1.03 --scene.date.on --scene.date.font.scale 0.03 --scene.date.font.color.foreground red --scene.date.font.color.background white',
  "Enable debug mode                                                " ,  debug               => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[0].scale 1.03 --window.debug.on',
  "Select palette automatically                                     " ,  bw                  => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[0].scale 1.03',
  "Cities                                                           " ,  cities              => '--landscape.on --cities.on --cities.points.scale 1.01 --cities.points.size.value 1 --cities.points.size.variable.on --view.lat 46.7 --view.lon 2 --view.fov 2 --cities.points.size.value 2 --cities.labels.on --cities.labels.font.color.foreground red --cities.labels.font.scale 0.04',
  "Colorbar                                                         " ,  colorbar            => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[0].scale 1.03 --colorbar.on --colorbar.font.color.foreground green --window.width 1000',
  "Low-res contour                                                  " ,  contour1            => '--window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path testdata/contour/t0049.grb --field[0].scale 1.03 --field[0].contour.on',
  "Medium-res contour                                               " ,  contour2            => '--window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path testdata/contour/t0479.grb --field[0].scale 1.03 --field[0].contour.on',
  "High-res contour                                                 " ,  contour3            => '--window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path testdata/contour/t1798.grb --field[0].scale 1.03 --field[0].contour.on',
  "Contour & raster, Lambert geometry                               " ,  contour_aro1        => '--field[0].path testdata/aro_small/S041WIND.U.PHYS.grb --field[0].scale 1.00 --field[0].palette.name cold_hot --field[1].path testdata/aro_small/S041WIND.U.PHYS.grb --field[1].scale 1.03 --field[1].contour.on --view.lon 26.64 --view.lat 67.36 --view.fov 0.5 --coast.on --grid.on',
  "Contour & raster, Lambert geometry                               " ,  contour_aro2        => '--field[0].path testdata/aro2.5/S090WIND.U.PHYS.grb --field[0].scale 1.00 --field[0].palette.name cold_hot --field[1].path testdata/aro2.5/S090WIND.U.PHYS.grb --field[1].scale 1.03 --field[1].contour.on --view.lat 46.2 --view.lon 2.0 --view.fov 5 --coast.on --grid.on',
  "Contour in diff mode, global lat/lon geometry                    " ,  contour_diff        => '--field[0].diff.on --field[0].contour.on --field[0].path testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0000.grib2 testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0102.grib2 --field[0].scale 1.01',
  "Low-res simple contour, limited area lat/lon geometry            " ,  contour_latlon1     => '--window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path testdata/contour/latlon9x6.grb --field[0].scale 1.03 --field[0].contour.on',
  "Low-res simple contour, limited area lat/lon geometry            " ,  contour_latlon2     => '--window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path testdata/contour/latlon18x13.grb --field[0].scale 1.03 --field[0].contour.on',
  "Medium-res contour, limited area lat/lon geometry                " ,  contour_latlon3     => '--window.width 1024 --window.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path testdata/eurat01/lfpw_0_0_0_pl_1000_t.grib2 --field[0].scale 1.03 --field[0].contour.on',
  "Medium-res contour and raster, global geometry                   " ,  contour_latlon4     => '--window.width 1024 --window.height 1024 --field[0].path testdata/glob025/lfpw_0_0_0_pl_1000_t.grib2 --field[1].path testdata/glob025/lfpw_0_0_0_pl_1000_t.grib2 --field[1].palette.name cold_hot --field[1].contour.on',
  "Contour on stretched/rotated gaussian geometry                   " ,  contour_stretched   => '--window.width 1024 --window.height 1024 --field[0].path testdata/t1198c2.2/Z.grb --field[0].scale 1.03 --field[0].contour.on',
  "Contour with dashed lines, gaussian geometry                     " ,  dashed1             => '--landscape.on --landscape.orography 0 --landscape.wireframe.on --landscape.geometry testdata/contour/t0479.grb --window.width 1024 --window.height 1024 --field[0].path testdata/contour/t0479.grb --field[0].scale 1.00 --field[0].contour.on --field[0].contour.lengths 100 --field[0].contour.patterns X- --view.fov 5',
  "Contour with dashed thick lines, gaussian geometry               " ,  dashed2             => '--landscape.on --landscape.orography 0 --landscape.wireframe.on --landscape.geometry testdata/contour/t0479.grb --window.width 1024 --window.height 1024 --field[0].path testdata/contour/t0479.grb --field[0].scale 1.01 --field[0].contour.on --field[0].contour.lengths 100 --field[0].contour.patterns XXXXXXXXX-X- --view.fov 5 --field[0].contour.widths 5',
  "Contour with dashed thick colored lines, gaussian geometry       " ,  dashed3             => '--landscape.on --landscape.orography 0 --landscape.wireframe.on --landscape.geometry testdata/contour/t0479.grb --window.width 1024 --window.height 1024 --field[0].path testdata/contour/t0479.grb --field[0].scale 1.01 --field[0].contour.on --field[0].contour.lengths 100 --field[0].contour.patterns XXXXXXXXX-X- --view.fov 5 --field[0].contour.widths 5 --field[0].contour.colors red green blue',
  "Display French departements                                      " ,  departements        => '--departements.on --departements.lines.color blue --departements.lines.selector "code_insee = 23 or code_insee = 19 or code_insee = 87" --view.lon 2 --view.lat 46.7 --view.fov 5 --coast.on',
  "Temperature field on Europe, over global cloud fields            " ,  eurat01             => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[1].path testdata/eurat01/lfpw_0_0_0_pl_1000_t.grib2 --field[0].scale 1.02 --field[1].scale 1.03 --field[0].palette.name cloud_auto --field[1].palette.name cold_hot',
  "Test FA format, display sea fraction                             " ,  fa                  => '--field[0].path testdata/fa/PGD.t479.fa%SFX.FRAC_SEA --field[0].scale 1.03',
  "Surface temperature, global lat/lon field                        " ,  glob01              => '--field[0].path testdata/glob01/lfpw_0_0_0_sfc_0_t.grib2 --field[0].scale 1.00 --field[0].palette.name cold_hot_temp --coast.on --grid.on',
  "Lambert geometry, French Guyana                                  " ,  guyane              => '--landscape.on --field[0].path testdata/aro_guyane/SURFTEMPERATURE.grb --field[0].palette.name cold_hot_temp --field[0].scale 1.01 --view.lat 5 --view.lon -51 --view.fov 3 --coast.on --grid.on',
  "Image on top of cloud fields                                     " ,  image               => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[0].scale 1.03 --scene.image.on --scene.image.path testdata/image/B.bmp --scene.image.x0 0. --scene.image.y0 0. --scene.image.x1 0.1 --scene.image.y1 0.1',
  "Interpolate field over time                                      " ,  interpolation       => '--field[0].path testdata/interp/01.grb testdata/interp/36.grb --field[0].scale 1.03 --field[0].palette.name cold_hot_temp --view.lon 2 --view.lat 46.7 --view.fov 5 --scene.interpolation.on --scene.interpolation.frames 200 --window.width 1000 --window.height 1000 --scene.date.font.scale 0.03 --scene.date.font.color.foreground red --scene.date.on --coast.on --grid.on',
  "Landscape over Europe only                                       " ,  landscape_eurat01   => '--landscape.on --landscape.geometry testdata/eurat01/lfpw_0_0_0_pl_1000_t.grib2 --landscape.orography 0 --coast.on --grid.on',
  "Display mapscale                                                 " ,  mapscale            => '--landscape.on --mapscale.on',
  "Display contour field using Mercator projection                  " ,  mercator            => '--field[0].diff.on --field[0].contour.on --field[0].path testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0000.grib2 testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0102.grib2 --field[0].scale 1.01 --view.projection MERCATOR --landscape.on',
  "Display field with missing values                                " ,  missingvalue        => '--field[0].path testdata/t49/SFX.CLAY.grb --coast.on --grid.on',
  "Display field with keeping field values in RAM                   " ,  novalue             => '--landscape.on --landscape.geometry testdata/t1798/Z.grb --field[0].path testdata/t1798/SURFNEBUL.BASSE.grb --field[0].scale 1.03 --field[0].palette.name cloud_auto --field[0].no_value_pointer.on',
  "Display field in offscreen mode                                  " ,  offscreen           => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --window.offscreen.on --window.offscreen.frames 10 --scene.light.rotate.on --scene.light.on --field[0].path testdata/t1198c2.2/SURFNEBUL.BASSE.grb testdata/t1198c2.2/SURFNEBUL.MOYENN.grb testdata/t1198c2.2/SURFNEBUL.HAUTE.grb --field[0].scale 1.03 1.03 1.03 --field[0].palette.name cloud_auto cloud_auto cloud_auto',
  "Display field with options in file                               " ,  optionsfile         => '--{testdata/options.list}',
  "Display field with palette gradient color                        " ,  palette_values_grad => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[0].scale 1.03 --field[0].palette-{ --colors "#00000000" "#008bff" "#01f8e9" "#05cf66" "#34c00c" "#b6e904" "#ffe600" "#ffb500" "#ff6900" "#ff0f00" "#b3003e" "#570088" --values 0 2 6 10 14 18 22 26 30 34 38 42 --min 0 --max 46 }- --colorbar.on --window.width 1200',
  "Display field with discrete palette                              " ,  palette_values      => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[0].scale 1.03 --field[0].palette-{ --colors "#00000000" "#008bff" "#01f8e9" "#05cf66" "#34c00c" "#b6e904" "#ffe600" "#ffb500" "#ff6900" "#ff0f00" "#b3003e" "#570088" --values 0 2 6 10 14 18 22 26 30 34 38 42 46 --min 0 --max 46 }- --colorbar.on --window.width 1200',
  "Display field difference                                         " ,  scalar_diff         => '--field[0].diff.on --field[0].path testdata/interp/01.grb testdata/interp/36.grb --field[0].scale 1.01 --view.lon 2 --view.lat 46.7 --view.fov 5 --field[0].palette-{ --name cold_hot --min -10 --max +10 }-',
  "Select field from GRIB file                                      " ,  select_grib         => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb%"parameterCategory=6,year=2019" --field[0].scale 1.03',
  "Read commands from a file                                        " ,  shell_batch         => 'testdata/glgrib.in',
  "Test interactive shell                                           " ,  shell               => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[0].scale 1.03 --shell.on',
  "Wind on small AROME domain (raster & vector)                     " ,  small_aro           => '--field[0].vector.on --field[0].path testdata/aro_small/S041WIND.U.PHYS.grb testdata/aro_small/S041WIND.V.PHYS.grb --field[0].scale 1.00 --view.lon 26.64 --view.lat 67.36 --view.fov 0.5 --coast.on --grid.on',
  "Low-res ARPEGE resolution                                        " ,  small               => '--landscape.on --landscape.geometry testdata/t49/Z.grb --field[0].scale 1.01 --field[0].path testdata/t49/SFX.CLAY.grb --coast.on --grid.on',
  "3D strings on the sphere                                         " ,  strxyz              => '--field[0].path testdata/t1198c2.2/N.grb --field[0].scale 0.99 --colorbar.on --colorbar.font.color.foreground green --window.width 1000 --view.projection XYZ --scene.test_strxyz.on',
  "Display text                                                     " ,  text                => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[0].scale 1.03 --scene.text.on --scene.text.s "coucou" --scene.text.x 0.0 --scene.text.y 1.0 --scene.text.a NW --scene.text.font.scale 0.03 --scene.text.font.color.foreground black --scene.text.font.color.background white',
  "Display title (field name)                                       " ,  title               => '--landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[0].scale 1.03 --scene.title.on',
  "Test travelling option                                           " ,  travelling          => '--scene.travelling.on --scene.travelling.pos1.lon 0 --scene.travelling.pos1.lat 0 --scene.travelling.pos1.fov 30 --scene.travelling.pos2.lon 90 --scene.travelling.pos2.lat 45 --scene.travelling.pos2.fov 10 --landscape.on --landscape.geometry testdata/t1198c2.2/Z.grb --field[0].path testdata/t1198c2.2/N.grb --field[0].scale 1.03',
  "Display vector norm & arrow, Lambert geometry                    " ,  vector              => '--field[0].vector.on --field[0].path testdata/aro2.5/S090WIND.U.PHYS.grb testdata/aro2.5/S090WIND.V.PHYS.grb --field[0].scale 1.00 --field[0].vector.color green --view.lat 46.2 --view.lon 2.0 --view.fov 5 --coast.on --grid.on',
  "Display vector norm, global lat/lon geometry                     " ,  vector_glob25       => '--landscape.on --landscape.geometry testdata/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 --landscape.orography 0 --field[0].vector.on --field[0].path testdata/arpt1798_wind/glob25_+1.grb testdata/arpt1798_wind/glob25_+1.grb --field[0].scale 1.01 --coast.on --grid.on',
  "Vector arrow, stretched/rotated gaussian geometry                " ,  vector_t1798        => '--landscape.on --landscape.geometry testdata/arpt1798_wind/+1.grb --landscape.orography 0 --field[0].vector.on --field[0].path testdata/arpt1798_wind/+1.grb testdata/arpt1798_wind/+1.grb --field[0].scale 1.01 --coast.on --grid.on',
  "Display landscape in WebMercator projection                      " ,  webmercator         => '--landscape-{ --on --path ./landscape/WebMercator_00006_00021_00028_00037_00035.ORTHOIMAGERY.ORTHOPHOTOS.bmp --projection WEBMERCATOR }- --grid.on',
  "Display wind on stretched/rotated gaussian geometry              " ,  wind_arp            => '--landscape.on --landscape.geometry testdata/t31c2.4/Z.grb --field[0].vector.on --field[0].path testdata/t31c2.4/S015WIND.U.PHYS.grb testdata/t31c2.4/S015WIND.V.PHYS.grb --field[0].scale 1.01 --coast.on --grid.on',
  "Wind on global lat/lon geometry                                  " ,  wind_glob25         => '--landscape.on --landscape.geometry testdata/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 --landscape.orography 0 --field[0].vector.on --field[0].path testdata/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 testdata/arpt1798_wind/lfpw_0_2_3_sfc_20_v.grib2 --field[0].scale 1.01 --coast.on --grid.on',
  "Wind on stretched/rotated gaussian geometry                      " ,  wind_t1798          => '--landscape.on --landscape.geometry testdata/arpt1798_wind/S105WIND.U.PHYS.grb --landscape.orography 0 --field[0].vector.on --field[0].path testdata/arpt1798_wind/S105WIND.U.PHYS.grb testdata/arpt1798_wind/S105WIND.V.PHYS.grb --field[0].scale 1.01 --coast.on --grid.on',
  "Streamlines on t1198c2.2 (surface)                               " ,  stream_t1198c22     => '--field[0].scale 1.01 --field[0].path testdata/wind+temp/t1198c2.2/S105WIND.U.PHYS.grb testdata/wind+temp/t1198c2.2/S105WIND.V.PHYS.grb --field[0].stream.on --field[0].palette-{ --colors grey green --min 0. --max 40. --values 0. 40. }- --field[0].stream.width 1.0 --field[0].stream.density 1.0 --field[1].scale 1.01 --field[1].path testdata/wind+temp/t1198c2.2/S105WIND.U.PHYS.grb testdata/wind+temp/t1198c2.2/S105WIND.V.PHYS.grb --field[1].vector.on --field[1].vector.hide_norm.on --field[1].vector.color red --grid.on --grid.color red --grid.scale 1.02 --coast.lines.scale 1.02 --coast.on --coast.lines.color red --landscape.on --landscape.path landscape/white.png',
  "Streamlines on GLOB025 (high level)                              " ,  stream_glob025      => '--field[0].path testdata/wind+temp/glob025/P020WIND.U.PHYS.grb testdata/wind+temp/glob025/P020WIND.V.PHYS.grb --field[0].stream.on --field[0].palette-{ --colors grey green --min 0. --max 40. --values 0. 40. }- --field[0].stream.width 1.0 --field[1].vector.on --field[1].path testdata/wind+temp/glob025/P020WIND.U.PHYS.grb testdata/wind+temp/glob025/P020WIND.V.PHYS.grb --field[1].vector.color red --field[1].vector.scale 5 --field[1].scale 1.01 --grid.on --landscape.on --landscape.path landscape/white.bmp --field[0].scale 1.01 --grid.color pink --coast.on --coast.lines.color pink',
  "Streamlines on EURAT01 (surface)                                 " ,  stream_eurat01      => '--field[0].path testdata/wind+temp/eurat01/H020WIND.U.PHYS.grb testdata/wind+temp/eurat01/H020WIND.V.PHYS.grb --field[0].stream.on --field[0].palette-{ --colors grey green --min 0. --max 40. --values 0. 40. }- --field[0].stream.width 1.0 --field[1].vector.on --field[1].path testdata/wind+temp/eurat01/H020WIND.U.PHYS.grb testdata/wind+temp/eurat01/H020WIND.V.PHYS.grb --field[1].vector.color red --field[1].vector.scale 5 --field[1].scale 1.01 --grid.on --landscape.on --landscape.path landscape/white.bmp --field[0].scale 1.01 --grid.color pink --coast.on --coast.lines.color pink --view.lat 45 --view.lon 5 --view.fov 10',
  "Streamlines on AROME 2.5km (surface)                             " ,  stream_lambert      => '--field[0].path testdata/aro2.5/S090WIND.U.PHYS.grb testdata/aro2.5/S090WIND.V.PHYS.grb --field[0].stream.on --field[0].palette-{ --colors grey green --min 0. --max 40. --values 0. 40. }- --field[0].stream.width 0.0 --field[0].stream.density 0.5 --field[0].stream.width 0.5 --field[1].vector.on --field[1].path testdata/aro2.5/S090WIND.U.PHYS.grb testdata/aro2.5/S090WIND.V.PHYS.grb --field[1].vector.color red --field[1].vector.scale 1 --field[1].scale 1.01 --grid.on --landscape.on --landscape.path landscape/white.bmp --field[0].scale 1.01 --grid.color pink --coast.on --coast.lines.color pink --view.lon 2 --view.lat 46.7 --view.fov 4',
  "Gaussian grid wireframe                                          " ,  gauss_wireframe     => '--field[0]-{ --path testdata/wind+temp/t0149c2.2/S105TEMPERATURE.grb --scalar.wireframe.on }-  --coast.on --view.lon 2 --view.lat 46.7 --view.fov 5',
  "Scalar field displayed using points                              " ,  points_scalar       => '--field[0]-{ --path testdata/wind+temp/t0224/S105TEMPERATURE.grb --scalar.points-{ --on --size.value 0.5 --size.variable.on --size.factor.off }-  }-  --coast.on --coast.lines.color green',
  "Scalar field with points (stretched/rotated grid)                " ,  points_scalar_rot   => '--field[0]-{ --scale 1.01 --path testdata/wind+temp/t0149c2.2/S105TEMPERATURE.grb --scalar.points-{ --on --size.value 0.2 --size.variable.on --size.factor.on }-  }-  --coast.on --coast.lines.color green --landscape.on',
);


my @t = @test;
while (my ($desc, $name, $opts) = splice (@t, 0, 3))
  {
    $desc =~ s/\s*$//o;
    $test{$name} = [$desc, [&quotewords ('\s+', 0, $opts)]];
  }


my $help = $ENV{HELP};

if ($help)
  {
    &help ();
  }

my $readme = $ENV{README};

if ($readme)
  {
    &readme ();
  }


my $name = $ENV{NAME} || '';

my @name = $name eq 'all' ? sort keys (%test) : ($name);

for my $name (@name)
  {

    my $auto = $ENV{AUTO};
    my $comp = $ENV{COMP};
    
    (my $test = $test{$name}) or die ("Unknown test `$name'\n");
    
    system ('make', 'glgrib.x')
      && die;
    
    my $exec = "./glgrib.x";
    
    my @args = @{ $test->[1] };
    
    if ($args[0] !~ m/^--/o)
      {
        $auto = 0;
      }
    
    if ($auto)
      {
        my $off = grep { m/^--window.offscreen.on$/o } @args;
        push (@args, '--window.offscreen.on') unless ($off);
        push (@args, '--window.offscreen.format', 'TEST_%N.png');
        for (<TEST*.png>)
          {
            unlink ($_);
          }
        @args = grep { $_ ne '--shell.on' } @args;
      }
    
    my @cmd = ('gdb', '-ex=set confirm on', '-ex=run', '-ex=quit', '--args', $exec, @args);
    
    print "Running test $name...\n"; 

    system (@cmd)
      and die;
    

    if ($auto)
      {
        &mkpath ("test.run/$name");
        
        for my $png (<TEST*.png>)
          {
            my $new = "test.run/$name/$png";
            my $thumb = "test.run/$name/thumb_$png";
            rename ($png, $new);
            system ('convert', -geometry => 300, $new, $thumb);
            my $ref = "test.ref/$name/$png";
            my $dif = "test.run/$name/diff_$png";
            if ((-f $ref) && ($comp))
              {
                'FileHandle'->new (">>test.run/test.log")->print ("$name\n");
                my @cmd = ('compare', '-metric', 'MAE', $ref, $new, $dif);
                print "@cmd\n";
		if (my $pid = fork ())
		  {
                    waitpid ($pid, 0);
		  }
		else
		  {
                    open (STDOUT, ">>test.run/test.log");
                    open (STDERR, ">>test.run/test.log");
                    exec (@cmd);
		  }
                print "\n";
                'FileHandle'->new (">>test.run/test.log")->print ("\n");
    	      }
          }
    
      }
        
  }
        
