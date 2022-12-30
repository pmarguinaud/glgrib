#!/usr/bin/perl -w
#
use strict;
use File::Path;
use Data::Dumper;
use Text::ParseWords;
use FileHandle;
use FindBin qw ($Bin);


my (%test, @test);

sub help
{
  my @t = @test;
  while (my ($kind, $desc, $name, $opts) = splice (@t, 0, 4))
    {
      print "* $name\n";
      print "   $desc\n";
      my @o = @{ $test{$name}[1][0] };
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
  my $doc = './share/doc/glgrib';
  my @t = @test;

  my ($fh, $fg, %fh) = ('FileHandle'->new (">README.md"), 'FileHandle'->new (">$doc/GLGRIB.md"));

# my $ext = '?raw=true';
  my $ext = '';

  for ($fh, $fg)
    {
      $_->print ("
Display GRIB2 fields with OpenGL. Raster, contour, vector, colorbar, mapscale, coastlines, borders. Lat/lon, lambert, gaussian grid.
GLFW backend for interactive display, EGL backend for batch processing without X11 display.

Perl/Tk interface.

Tested on :

* VGA compatible controller: Intel Corporation Xeon E3-1200 v3/4th Gen Core Processor Integrated Graphics Controller (rev 06) 
* VGA compatible controller: Intel Corporation HD Graphics 530 (rev 06) 
* VGA compatible controller: Advanced Micro Devices, Inc. [AMD/ATI] Wani [Radeon R5/R6/R7 Graphics] (rev c8) 
* 3D controller: NVIDIA Corporation GV100GL [Tesla V100S PCIe 32GB] (rev a1) 

  ");

  $fh->print ("

![]($doc/perltk/tk1.png$ext)
![]($doc/perltk/tk2.png$ext)

  ");

  $fg->print ("

![](./perltk/tk1.png$ext)
![](./perltk/tk2.png$ext)

  ");


  while (my ($kind, $desc, $name, $opts) = splice (@t, 0, 4))
    {

      $opts = join (' ', @{ $opts->[0] });

      next unless ($opts =~ m/^--/o);

      my $title = "## $desc -- $name\n";
 
      my ($link1, $link2);

      if (-f "$doc/samples/$name/TEST.gif$ext")
        {
          $link1 = "![]($doc/samples/$name/TEST.gif$ext)\n";
          $link2 = "![](samples/$name/TEST.gif$ext)\n";
        }
      else
        {
          $link1 .= "![]($doc/samples/$name/TEST_0000.png$ext)\n";
          $link2 .= "![](samples/$name/TEST_0000.png$ext)\n";
        }

      my $opts = "\n";

      my @o = @{ $test{$name}[1][0] };

      my @l = ('');
      while (defined (my $o = shift (@o)))
        {
          push @l, '' if (length ($l[-1]) + length ($o) > 70);
          $l[-1] .= "$o ";
        }

      $opts .= "```\n";
      for (@l)
        {
          $opts .= "    $_\n";
        }
      $opts .= "```\n";

      my $text1 = "$title$link1$opts";
      my $text2 = "$title$link2$opts";

      unless ($fh{$kind})
        {
          my $Kind = ucfirst (lc ($kind));
          $fh{$kind} = 'FileHandle'->new (">$doc/$kind.md");

          $fh->print ("# [$Kind ...]($doc/$kind.md)\n");
          $fh->print ($text1);

          $fg->print ("# [$Kind ...]($kind.md)\n");
          $fg->print ($text2);

        }

      $fh{$kind}->print ($text2);

    }
  die ("\n");
}


@test =
(
 'CLOUD'        , "Clouds, 3 layers, t1198c2.2                                      " ,  t1198_3l            => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/SURFNEBUL.BASSE.grb --field[1].path share/glgrib/testdata/t1198c2.2/SURFNEBUL.MOYENN.grb --field[2].path share/glgrib/testdata/t1198c2.2/SURFNEBUL.HAUTE.grb --field[0].scale 1.03 --field[1].scale 1.04 --field[2].scale 1.05 --field[0].palette.name cloud --field[1].palette.name cloud --field[2].palette.name cloud',
 'CLOUD'        , "Clouds, 3 layers, t1798c2.2                                      " ,  t1798_3l            => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1798/Z.grb --field[0].path share/glgrib/testdata/t1798/SURFNEBUL.BASSE.grb --field[1].path share/glgrib/testdata/t1798/SURFNEBUL.MOYENN.grb --field[2].path share/glgrib/testdata/t1798/SURFNEBUL.HAUTE.grb --field[0].scale 1.03 --field[1].scale 1.04 --field[2].scale 1.05 --field[0].palette.name cloud --field[1].palette.name cloud --field[2].palette.name cloud',
 'LAMBERT'      , "AROME 1.3km                                                      " ,  arome13             => '--field[0].path share/glgrib/testdata/aro1.3/CLSVENT.ZONAL.grb share/glgrib/testdata/aro1.3/CLSVENT.MERIDIEN.grb --field[0].type VECTOR  --view-{ --lon 2 --lat 46.2 --fov 3 }- --field[0].palette.colors black green --field[0].palette.values 0.  20.  --field[0].vector.arrow.color red --coast.lines.color cyan --coast.on',
 'LAMBERT'      , "AROME, Lambert geometry                                          " ,  aro                 => '--field[0].path share/glgrib/testdata/aro2.5/SURFIND.TERREMER.grb --field[0].palette.name cold_hot --field[0].scale 1.00 --view.lat 46.2 --view.lon 2.0 --view.fov 5 --coast.on --grid.on',
 'LAMBERT'      , "AROME, Lambert geometry, polar stereo                            " ,  aro_polar           => '--field[0].path share/glgrib/testdata/aro2.5/SURFIND.TERREMER.grb --field[0].palette.name cold_hot --field[0].scale 1.00 --view.lat 46.2 --view.lon 2.0 --view.fov 5 --coast.on --grid.on --view.projection POLAR_NORTH --view.lat 70 --view.fov 12',
 'STRING'       , "Contour with labels                                              " ,  contourlabels2      => '--landscape.on --field[0].path share/glgrib/testdata/z500.grb --field[0].scale 1.01 --field[0].type CONTOUR --view.lat 56 --view.lon 20 --field[0].contour.labels.on --grid.on --grid.color black --field[0].palette.colors red  --field[0].contour.levels 48000 48500 49000 49500 50000 50500 51000 51500 52000 52500 53000 53500 54000 54500 55000 55500 56000 56500 57000 57500 --field[0].contour.labels-{ --font.color.foreground white --font.color.background black --font.bitmap.scale 0.04 --format \'%.0f\' }- --coast.on --coast.lines.color pink',
 'STRING'       , "Font background color                                            " ,  background          => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[0].scale 1.03 --scene.date.on --scene.date.font.bitmap.scale 0.03 --scene.date.font.color.foreground red --scene.date.font.color.background white',
 'MISC'         , "Cities                                                           " ,  cities              => '--landscape.on --cities.on --cities.points.scale 1.01 --cities.points.size.value 1 --cities.points.size.variable.on --view.lat 46.7 --view.lon 2 --view.fov 2 --cities.points.size.value 2 --cities.labels.on --cities.labels.font.color.foreground red --cities.labels.font.bitmap.scale 0.04',
 'MISC'         , "Enable debug mode                                                " ,  debug               => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[0].scale 1.03 --render.debug.on',
 'PALETTE'      , "Display field with palette gradient color                        " ,  palette_values_grad => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[0].scale 1.03 --field[0].palette-{ --colors "#00000000" "#008bff" "#01f8e9" "#05cf66" "#34c00c" "#b6e904" "#ffe600" "#ffb500" "#ff6900" "#ff0f00" "#b3003e" "#570088" --values 0 2 6 10 14 18 22 26 30 34 38 42 --min 0 --max 46 }- --colorbar.on --render.width 1200',
 'PALETTE'      , "Select palette automatically                                     " ,  bw                  => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[0].scale 1.03',
 'MISC'         , "Colorbar                                                         " ,  colorbar            => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[0].scale 1.03 --colorbar.on --colorbar.font.color.foreground green --render.width 1000',
 'CONTOUR'      , "Medium-res contour and raster, global geometry                   " ,  contour_latlon4     => '--render.width 1024 --render.height 1024 --field[0].path share/glgrib/testdata/glob025/lfpw_0_0_0_pl_1000_t.grib2 --field[1].path share/glgrib/testdata/glob025/lfpw_0_0_0_pl_1000_t.grib2 --field[1].palette.colors black --field[1].type CONTOUR --field[1].scale 1.001',
 'CONTOUR'      , "Low-res contour                                                  " ,  contour1            => '--render.width 1024 --render.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path share/glgrib/testdata/contour/t0049.grb --field[0].scale 1.03 --field[0].type CONTOUR',
 'CONTOUR'      , "Medium-res contour                                               " ,  contour2            => '--render.width 1024 --render.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path share/glgrib/testdata/contour/t0479.grb --field[0].scale 1.03 --field[0].type CONTOUR',
 'CONTOUR'      , "High-res contour                                                 " ,  contour3            => '--render.width 1024 --render.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path share/glgrib/testdata/contour/t1798.grb --field[0].scale 1.03 --field[0].type CONTOUR',
 'CONTOUR'      , "Contour & raster, Lambert geometry                               " ,  contour_aro1        => '--field[0].path share/glgrib/testdata/aro_small/S041WIND.U.PHYS.grb --field[0].scale 1.00 --field[0].palette.name cold_hot --field[1].path share/glgrib/testdata/aro_small/S041WIND.U.PHYS.grb --field[1].scale 1.03 --field[1].type CONTOUR --view.lon 26.64 --view.lat 67.36 --view.fov 0.5 --coast.on --grid.on',
 'CONTOUR'      , "Contour & raster, Lambert geometry                               " ,  contour_aro2        => '--field[0].path share/glgrib/testdata/aro2.5/S090WIND.U.PHYS.grb --field[0].scale 1.00 --field[0].palette.name cold_hot --field[1].path share/glgrib/testdata/aro2.5/S090WIND.U.PHYS.grb --field[1].scale 1.03 --field[1].type CONTOUR --view.lat 46.2 --view.lon 2.0 --view.fov 5 --coast.on --grid.on',
 'DIFF'         , "Contour in diff mode, global lat/lon geometry                    " ,  contour_diff        => '--field[0].diff.on --field[0].type CONTOUR --field[0].path share/glgrib/testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0000.grib2 share/glgrib/testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0102.grib2 --field[0].scale 1.01 --landscape.on --landscape.color white  --field[0].user_pref.off',
 'CONTOUR'      , "Low-res simple contour, limited area lat/lon geometry            " ,  contour_latlon1     => '--render.width 1024 --render.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path share/glgrib/testdata/contour/latlon9x6.grb --field[0].scale 1.03 --field[0].type CONTOUR',
 'CONTOUR'      , "Low-res simple contour, limited area lat/lon geometry            " ,  contour_latlon2     => '--render.width 1024 --render.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path share/glgrib/testdata/contour/latlon18x13.grb --field[0].scale 1.03 --field[0].type CONTOUR',
 'CONTOUR'      , "Medium-res contour, limited area lat/lon geometry                " ,  contour_latlon3     => '--render.width 1024 --render.height 1024 --landscape.on --landscape.path landscape/black.bmp --field[0].path share/glgrib/testdata/eurat01/lfpw_0_0_0_pl_1000_t.grib2 --field[0].scale 1.03 --field[0].type CONTOUR',
 'CONTOUR'      , "Contour on stretched/rotated gaussian geometry                   " ,  contour_stretched   => '--render.width 1024 --render.height 1024 --field[0].path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].scale 1.03 --field[0].type CONTOUR',
 'CONTOUR'      , "Contour with dashed lines, gaussian geometry                     " ,  dashed1             => '--landscape.on --landscape.wireframe.on --landscape.grid.path share/glgrib/testdata/contour/t0479.grb --render.width 1024 --render.height 1024 --field[0].palette.colors white --field[0].path share/glgrib/testdata/contour/t0479.grb --field[0].scale 1.00 --field[0].type CONTOUR --field[0].contour.lengths 100 --field[0].contour.patterns X- --view.fov 5',
 'CONTOUR'      , "Contour with dashed thick lines, gaussian geometry               " ,  dashed2             => '--landscape.on --landscape.wireframe.on --landscape.grid.path share/glgrib/testdata/contour/t0479.grb --render.width 1024 --render.height 1024 --field[0].palette.colors white --field[0].path share/glgrib/testdata/contour/t0479.grb --field[0].scale 1.01 --field[0].type CONTOUR --field[0].contour.lengths 100 --field[0].contour.patterns XXXXXXXXX-X- --view.fov 5 --field[0].contour.widths 5',
 'CONTOUR'      , "Contour with dashed thick colored lines, gaussian geometry       " ,  dashed3             => '--landscape.on --landscape.wireframe.on --landscape.grid.path share/glgrib/testdata/contour/t0479.grb --render.width 1024 --render.height 1024 --field[0].path share/glgrib/testdata/contour/t0479.grb --field[0].scale 1.01 --field[0].type CONTOUR --field[0].contour.lengths 100 --field[0].contour.patterns XXXXXXXXX-X- --view.fov 5 --field[0].contour.widths 5 --field[0].palette.colors red green blue',
 'MISC'         , "Display French departements                                      " ,  departements        => '--departements.on --departements.lines.color blue --departements.lines.selector "code_insee = 23 or code_insee = 19 or code_insee = 87" --view.lon 2 --view.lat 46.7 --view.fov 5 --coast.on',
 'CLOUD'        , "Temperature field on Europe, over global cloud fields            " ,  eurat01             => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[1].path share/glgrib/testdata/eurat01/lfpw_0_0_0_pl_1000_t.grib2 --field[0].scale 1.02 --field[1].scale 1.03 --field[0].palette.name cloud_auto --field[1].palette.name cold_hot',
 'MISC'         , "Test FA format, display sea fraction                             " ,  fa                  => '--field[0].path share/glgrib/testdata/fa/PGD.t479.fa%SFX.FRAC_SEA --field[0].scale 1.03',
 'SCALAR'       , "Surface temperature, global lat/lon field                        " ,  glob01              => '--field[0].path share/glgrib/testdata/glob01/lfpw_0_0_0_sfc_0_t.grib2 --field[0].scale 1.00 --field[0].palette.name cold_hot_temp --coast.on --grid.on',
 'SCALAR'       , "Lambert geometry, French Guyana                                  " ,  guyane              => '--landscape.on --field[0].user_pref.off --field[0].path share/glgrib/testdata/aro_guyane/SURFTEMPERATURE.grb --field[0].palette.name cold_hot_temp --field[0].scale 1.01 --view.lat 5 --view.lon -51 --view.fov 3 --coast.on --grid.on',
 'MISC'         , "Image on top of cloud fields                                     " ,  image               => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[0].scale 1.03 --scene.image.on --scene.image.path share/glgrib/testdata/image/B.bmp --scene.image.x0 0. --scene.image.y0 0. --scene.image.x1 0.1 --scene.image.y1 0.1',
 'MISC'         , "Interpolate field over time                                      " ,  interpolation       => '--field[0].path share/glgrib/testdata/interp/01.grb share/glgrib/testdata/interp/36.grb --field[0].scale 1.03 --field[0].palette.name cold_hot_temp --view.lon 2 --view.lat 46.7 --view.fov 5 --scene.interpolation.on --scene.interpolation.frames 200 --render.width 1000 --render.height 1000 --scene.date.font.bitmap.scale 0.03 --scene.date.font.color.foreground red --scene.date.on --coast.on --grid.on  --render.offscreen.on --render.offscreen.frames 72',
 'LANDSCAPE'    , "Landscape over Europe only                                       " ,  landscape_eurat01   => '--landscape.on --landscape.grid.path share/glgrib/testdata/eurat01/lfpw_0_0_0_pl_1000_t.grib2 --coast.on --grid.on',
 'MISC'         , "Display mapscale                                                 " ,  mapscale            => '--landscape.on --mapscale.on',
 'PROJECTION'   , "Display contour field using Mercator projection                  " ,  mercator            => '--field[0].diff.on --field[0].user_pref.off --field[0].type CONTOUR --field[0].path share/glgrib/testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0000.grib2 share/glgrib/testdata/glob01/lfpw_0_3_1_sfc_0_prmsl+0102.grib2 --field[0].scale 1.01 --view.projection MERCATOR --landscape.on',
 'PROJECTION'   , "Display contour field using different origin                     " ,  mercator_origin     => '--field[0].path ./share/glgrib/testdata/wind+temp/t0224/S105TEMPERATURE.grb  --grid.on --coast.on --coast.lines.color black --view.projection MERCATOR --view.transformation PERSPECTIVE --view.lon 0 --view.lat -45 --view.roll 0 --view.pitch 0 --view.yaw 0 --view.fov 20 --view.distance 6 --view.center.on --view.clip  --view.clip.on --view.clip.dlon 10 --view.clip.dlat 5 --view.clip.xmin 0 --view.clip.xmax 1 --view.clip.ymin 0 --view.clip.ymax 1 --view.zoom  --view.zoom.off --view.zoom.lon 2 --view.zoom.lat 46.7 --view.zoom.stretch 0.5 --view.trans.on --view.trans.matrix 0.707106781186548 0 0.707106781186548 0 1 0 -0.707106781186548 0 0.707106781186548',
 'PROJECTION'   , "Display contour field using different origin                     " ,  polar_origin        => '--field[0].path ./share/glgrib/testdata/wind+temp/t0224/S105TEMPERATURE.grb  --grid.on --coast.on --coast.lines.color black --render.width 1200 --render.height 800  --view.projection POLAR_NORTH --view.transformation PERSPECTIVE --view.lon -73.4742 --view.lat 43.9614 --view.roll 0 --view.pitch 0 --view.yaw -2 --view.fov 21 --view.distance 6 --view.center.on --view.clip  --view.clip.on --view.clip.dlon 10 --view.clip.dlat 5 --view.clip.xmin 0 --view.clip.xmax 1 --view.clip.ymin 0 --view.clip.ymax 1 --view.zoom  --view.zoom.off --view.zoom.lon 2 --view.zoom.lat 46.7 --view.zoom.stretch 0.5 --view.trans.on --view.trans.matrix 0.707106781186548 0 0.707106781186548 0 1 0 -0.707106781186548 0 0.707106781186548',
 'MISC'         , "Display field with missing values                                " ,  missingvalue        => '--field[0].path share/glgrib/testdata/t49/SFX.CLAY.grb --coast.on --grid.on --field[0].user_pref.off',
 'MISC'         , "Display field with keeping field values in RAM                   " ,  novalue             => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1798/Z.grb --field[0].path share/glgrib/testdata/t1798/SURFNEBUL.BASSE.grb --field[0].scale 1.03 --field[0].palette.name cloud_auto --field[0].no_value_pointer.on',
 'MISC'         , "Display field in offscreen mode                                  " ,  offscreen           => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --render.offscreen.on --render.offscreen.frames 36 --scene.light.rotate.on --scene.light.on --field[0].path share/glgrib/testdata/t1198c2.2/SURFNEBUL.BASSE.grb share/glgrib/testdata/t1198c2.2/SURFNEBUL.MOYENN.grb share/glgrib/testdata/t1198c2.2/SURFNEBUL.HAUTE.grb --field[0].scale 1.03 1.03 1.03 --field[0].palette.name cloud_auto cloud_auto cloud_auto --scene.light.rotate.rate 10',
 'MISC'         , "Display field with options in file                               " ,  optionsfile         => '--{share/glgrib/testdata/options.list}',
 'PALETTE'      , "Display field with discrete palette                              " ,  palette_values      => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[0].scale 1.03 --field[0].palette-{ --colors "#00000000" "#008bff" "#01f8e9" "#05cf66" "#34c00c" "#b6e904" "#ffe600" "#ffb500" "#ff6900" "#ff0f00" "#b3003e" "#570088" --values 0 2 6 10 14 18 22 26 30 34 38 42 46 --min 0 --max 46 }- --colorbar.on --render.width 1200',
 'DIFF'         , "Display field difference                                         " ,  scalar_diff         => '--field[0].diff.on --field[0].path share/glgrib/testdata/interp/01.grb share/glgrib/testdata/interp/36.grb --field[0].scale 1.01 --view.lon 2 --view.lat 46.7 --view.fov 5 --field[0].palette-{ --name cold_hot --min -10 --max +10 }-',
 'MISC'         , "Select field from GRIB file                                      " ,  select_grib         => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb%"parameterCategory=6,year=2019" --field[0].scale 1.03',
 'SHELL'        , "Read commands from a file                                        " ,  shell_batch         => 'share/glgrib/testdata/glgrib.in',
 'SHELL'        , "Test interactive shell                                           " ,  shell               => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[0].scale 1.03 --shell.on',
 'VECTOR'       , "Wind Australia                                                   " ,  windaustralia3      => '--field[0].path share/glgrib/testdata/uv200.grib%paramId=131 share/glgrib/testdata/uv200.grib%paramId=132 --field[0].type VECTOR --coast.on  --view.lat -37 --view.lon 140 --view.fov 3 --field[0].vector.norm.off --land.on  --landscape.on --landscape.color "#333333" --landscape.scale 0.999 --coast.on --coast.lines.color black --coast.lines.path "share/glgrib/coastlines/gshhg/GSHHS_bin/gshhs_i.b"  --field[0].vector.scale 1  --render.width 1200 --field[0].scale 1.001 --view.projection LATLON  --field[0].vector.density 10 --field[0].palette.colors "#1cb8a6" "#19c25c" "#24cb15" "#80d511" "#e0d50d" "#ea7209" "#f50408" "#ff007f" --field[0].palette.values 20 30 40 50 60 70 80 90 100 --field[0].vector.arrow.color "#00000000" --field[0].vector.arrow.fixed.on  --field[0].vector.arrow.min 10  --land.layers[0].color black --coast.on --coast.lines.color "#555555" --colorbar.on --field[0].vector.density 20 --field[0].vector.scale 2',
 'VECTOR'       , "Wind on small AROME domain (raster & vector)                     " ,  small_aro           => '--field[0].type VECTOR --field[0].path share/glgrib/testdata/aro_small/S041WIND.U.PHYS.grb share/glgrib/testdata/aro_small/S041WIND.V.PHYS.grb --field[0].scale 1.00 --view.lon 26.64 --view.lat 67.36 --view.fov 0.5 --coast.on --grid.on',
 'MISC'         , "Low-res ARPEGE resolution                                        " ,  small               => '--landscape.on --landscape.grid.path share/glgrib/testdata/t49/Z.grb --field[0].scale 1.01 --field[0].path share/glgrib/testdata/t49/SFX.CLAY.grb --coast.on --grid.on --field[0].user_pref.off',
 'STRING'       , "Display text                                                     " ,  text                => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[0].scale 1.03 --scene.text.on --scene.text.s "coucou" --scene.text.x 0.0 --scene.text.y 1.0 --scene.text.a NW --scene.text.font.bitmap.scale 0.03 --scene.text.font.color.foreground black --scene.text.font.color.background white',
 'STRING'       , "Display title (field name)                                       " ,  title               => '--landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[0].scale 1.03 --scene.title.on',
 'MISC'         , "Test travelling option                                           " ,  travelling          => '--scene.travelling.on --scene.travelling.pos1.lon 0 --scene.travelling.pos1.lat 0 --scene.travelling.pos1.fov 30 --scene.travelling.pos2.lon 90 --scene.travelling.pos2.lat 45 --scene.travelling.pos2.fov 10 --landscape.on --landscape.grid.path share/glgrib/testdata/t1198c2.2/Z.grb --field[0].path share/glgrib/testdata/t1198c2.2/N.grb --field[0].scale 1.03 --render.offscreen.on --render.offscreen.frames 72',
 'VECTOR'       , "Display vector norm & arrow, Lambert geometry                    " ,  vector              => '--field[0].type VECTOR --field[0].path share/glgrib/testdata/aro2.5/S090WIND.U.PHYS.grb share/glgrib/testdata/aro2.5/S090WIND.V.PHYS.grb --field[0].scale 1.00 --field[0].vector.arrow.color green --view.lat 46.2 --view.lon 2.0 --view.fov 5 --coast.on --grid.on',
 'VECTOR'       , "Display vector norm, global lat/lon geometry                     " ,  vector_glob25       => '--landscape.on --landscape.grid.path share/glgrib/testdata/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 --field[0].type VECTOR --field[0].path share/glgrib/testdata/arpt1798_wind/glob25_+1.grb share/glgrib/testdata/arpt1798_wind/glob25_+1.grb --field[0].scale 1.01 --coast.on --grid.on',
 'VECTOR'       , "Vector arrow, stretched/rotated gaussian geometry                " ,  vector_t1798        => '--landscape.on --landscape.grid.path share/glgrib/testdata/arpt1798_wind/+1.grb --field[0].type VECTOR --field[0].path share/glgrib/testdata/arpt1798_wind/+1.grb share/glgrib/testdata/arpt1798_wind/+1.grb --field[0].scale 1.01 --coast.on --grid.on',
 'LANDSCAPE'    , "Display landscape in WebMercator projection                      " ,  webmercator         => '--landscape-{ --on --path ./share/glgrib/landscape/WebMercator_00006_00021_00028_00037_00035.ORTHOIMAGERY.ORTHOPHOTOS.png --projection WEBMERCATOR }- --grid.on',
 'VECTOR'       , "Display wind on stretched/rotated gaussian geometry              " ,  wind_arp            => '--landscape.on --landscape.grid.path share/glgrib/testdata/t31c2.4/Z.grb --field[0].type VECTOR --field[0].path share/glgrib/testdata/t31c2.4/S015WIND.U.PHYS.grb share/glgrib/testdata/t31c2.4/S015WIND.V.PHYS.grb --field[0].scale 1.01 --coast.on --grid.on',
 'VECTOR'       , "Wind on global lat/lon geometry                                  " ,  wind_glob25         => '--landscape.on --landscape.grid.path share/glgrib/testdata/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 --field[0].type VECTOR --field[0].path share/glgrib/testdata/arpt1798_wind/lfpw_0_2_2_sfc_20_u.grib2 share/glgrib/testdata/arpt1798_wind/lfpw_0_2_3_sfc_20_v.grib2 --field[0].scale 1.01 --coast.on --grid.on',
 'VECTOR'       , "Wind on stretched/rotated gaussian geometry                      " ,  wind_t1798          => '--landscape.on --landscape.grid.path share/glgrib/testdata/arpt1798_wind/S105WIND.U.PHYS.grb --field[0].type VECTOR --field[0].path share/glgrib/testdata/arpt1798_wind/S105WIND.U.PHYS.grb share/glgrib/testdata/arpt1798_wind/S105WIND.V.PHYS.grb --field[0].scale 1.01 --coast.on --grid.on',
 'STREAM'       , "Streamlines on t1198c2.2 (surface)                               " ,  stream_t1198c22     => '--field[0].scale 1.01 --field[0].path share/glgrib/testdata/wind+temp/t1198c2.2/S105WIND.U.PHYS.grb share/glgrib/testdata/wind+temp/t1198c2.2/S105WIND.V.PHYS.grb --field[0].type STREAM --field[0].palette-{ --colors grey green --min 0. --max 40. --values 0. 40. }- --field[0].stream.width 1.0 --field[0].stream.density 1.0 --field[1].scale 1.01 --field[1].path share/glgrib/testdata/wind+temp/t1198c2.2/S105WIND.U.PHYS.grb share/glgrib/testdata/wind+temp/t1198c2.2/S105WIND.V.PHYS.grb --field[1].type VECTOR --field[1].vector.norm.off --field[1].vector.arrow.color red --grid.on --grid.color red --grid.scale 1.02 --coast.lines.scale 1.02 --coast.on --coast.lines.color red --landscape.on --landscape.path landscape/white.png',
 'STREAM'       , "Streamlines on GLOB025 (high level)                              " ,  stream_glob025      => '--field[0].path share/glgrib/testdata/wind+temp/glob025/P020WIND.U.PHYS.grb share/glgrib/testdata/wind+temp/glob025/P020WIND.V.PHYS.grb --field[0].type STREAM --field[0].palette-{ --colors grey green --min 0. --max 40. --values 0. 40. }- --field[0].stream.width 1.0 --field[1].type VECTOR --field[1].path share/glgrib/testdata/wind+temp/glob025/P020WIND.U.PHYS.grb share/glgrib/testdata/wind+temp/glob025/P020WIND.V.PHYS.grb --field[1].vector.arrow.color red --field[1].vector.scale 5 --field[1].scale 1.01 --grid.on --landscape.on --landscape.path landscape/white.bmp --field[0].scale 1.01 --grid.color pink --coast.on --coast.lines.color pink',
 'STREAM'       , "Streamlines on EURAT01 (surface)                                 " ,  stream_eurat01      => '--field[0].path share/glgrib/testdata/wind+temp/eurat01/H020WIND.U.PHYS.grb share/glgrib/testdata/wind+temp/eurat01/H020WIND.V.PHYS.grb --field[0].type STREAM --field[0].palette-{ --colors grey green --min 0. --max 40. --values 0. 40. }- --field[0].stream.width 1.0 --field[1].type VECTOR --field[1].path share/glgrib/testdata/wind+temp/eurat01/H020WIND.U.PHYS.grb share/glgrib/testdata/wind+temp/eurat01/H020WIND.V.PHYS.grb --field[1].vector.arrow.color red --field[1].vector.scale 5 --field[1].scale 1.01 --grid.on --landscape.on --landscape.path landscape/white.bmp --field[0].scale 1.01 --grid.color pink --coast.on --coast.lines.color pink --view.lat 45 --view.lon 5 --view.fov 10',
 'STREAM'       , "Streamlines on AROME 2.5km (surface)                             " ,  stream_lambert      => '--field[0].path share/glgrib/testdata/aro2.5/S090WIND.U.PHYS.grb share/glgrib/testdata/aro2.5/S090WIND.V.PHYS.grb --field[0].type STREAM --field[0].palette-{ --colors grey green --min 0. --max 40. --values 0. 40. }- --field[0].stream.width 0.0 --field[0].stream.density 0.5 --field[0].stream.width 0.5 --field[1].type VECTOR --field[1].path share/glgrib/testdata/aro2.5/S090WIND.U.PHYS.grb share/glgrib/testdata/aro2.5/S090WIND.V.PHYS.grb --field[1].vector.arrow.color red --field[1].vector.scale 1 --field[1].scale 1.01 --grid.on --landscape.on --landscape.path landscape/white.bmp --field[0].scale 1.01 --grid.color pink --coast.on --coast.lines.color pink --view.lon 2 --view.lat 46.7 --view.fov 4',
 'POINTS'       , "Gaussian grid wireframe                                          " ,  gauss_wireframe     => '--field[0]-{ --path share/glgrib/testdata/wind+temp/t0149c2.2/S105TEMPERATURE.grb --scalar.wireframe.on }-  --coast.on --view.lon 2 --view.lat 46.7 --view.fov 5',
 'POINTS'       , "Scalar field displayed using points                              " ,  points_scalar       => '--field[0]-{ --path share/glgrib/testdata/wind+temp/t0224/S105TEMPERATURE.grb --scalar.points-{ --on --size.value 0.5 --size.variable.on --size.factor.off }-  }-  --coast.on --coast.lines.color green',
 'POINTS'       , "Scalar field with points (stretched/rotated grid)                " ,  points_scalar_rot   => '--field[0]-{ --scale 1.01 --path share/glgrib/testdata/wind+temp/t0149c2.2/S105TEMPERATURE.grb --scalar.points-{ --on --size.value 0.2 --size.variable.on --size.factor.on }-  }-  --coast.on --coast.lines.color green --landscape.on',
 'SCALAR'       , "Scalar field on octahedral grid                                  " ,  gauss_octahedral    => '--field[0].path ./share/glgrib/testdata/ecmwf/ecmf_0_1_0_ml_137_q.grib2  --field[0].palette.colors gray cyan blue  --field[0].palette.values 0 0.005 0.03  --colorbar.on --render.width 1200',
 'LANDSCAPE'    , "Landscape with orography (T479)                                  " ,  land_orography      => '--landscape.on --landscape.grid.path ./share/glgrib/testdata/t479/Z.grb  --landscape.geometry.height.on --landscape.geometry.height.scale 0.2 --landscape.geometry.height.path ./share/glgrib/testdata/t479/Z.grb --landscape.flat.off',
 'SPECTRAL'     , "Spherical harmonic (T1198)                                       " ,  harmonic            => '--field[0].path share/glgrib/testdata/harmonics/SPEC.+0008.+0004.grb --field[0].palette-{ --colors blue green red  --values -1 0. 1 }- --field[0].geometry.height.on --field[0].geometry.height.scale 0.2 --view.fov 25 --view.lon 20 --scene.light.on',
 'SPECTRAL'     , "Spherical harmonic (T1198C2.2)                                   " ,  harmonic_stretch    => '--field[0].path share/glgrib/testdata/harmonics/SPEC.+0008.+0004_stretched.grb --field[0].palette-{ --colors blue green red  --values -1 0. 1 }- --field[0].geometry.height.on --field[0].geometry.height.scale 0.2 --view.fov 25 --view.lon 20 --scene.light.on',
 'SPECTRAL'     , "Spherical harmonic (T1198)                                       " ,  tharmonic1          => '--field[0].path share/glgrib/testdata/wind+temp/t1198/S105TEMPERATURE.grb --field[1].path share/glgrib/testdata/harmonics/SPEC.+0008.+0004.grb --field[1].scale 1.01 --field[1].type CONTOUR --field[1].palette.colors black white --view.lon -25 --view.lat 25 --scene.light.on  --grid.on',
 'SPECTRAL'     , "Spherical harmonic (T1198)                                       " ,  tharmonic2          => '--field[0].path share/glgrib/testdata/wind+temp/t1198/S105TEMPERATURE.grb  --field[0].geometry.height.on --field[0].geometry.height.scale 0.2 --field[0].geometry.height.path share/glgrib/testdata/harmonics/SPEC.+0008.+0004.grb --field[0].scale 0.8 --scene.light.on  --grid.on  --view.lon -25 --view.lat 25',
 'SPECTRAL'     , "Spherical harmonic (T1198)                                       " ,  tharmonic3          => '--field[0].path share/glgrib/testdata/wind+temp/t1198/S105TEMPERATURE.grb --field[1].path share/glgrib/testdata/harmonics/SPEC.+0008.+0004.grb --field[1].scale 1.01 --field[1].palette.colors #00ff0044 #00000100 #00ff0044 --field[1].palette.values -1 0. +1 --view.lon -25 --view.lat 25 --scene.light.on  --grid.on',
 'SPECTRAL'     , "Spherical harmonic (T1198)                                       " ,  tharmonic4          => sub
  {
    my $i = shift;
    return unless ($i <= 40);
    my $f = sprintf ('SPEC.+0020.%+4.4d.grb', $i-20);
    return sprintf ('--field[0].path share/glgrib/testdata/wind+temp/t1198/S105TEMPERATURE.grb  --field[0].geometry.height.on '
                  . '--field[0].geometry.height.scale 0.2 --field[0].geometry.height.path share/glgrib/testdata/wind+temp/t1198/spectral/%s '
                  . '--field[0].scale 0.8 --scene.light.on  --grid.on  --view.lon -25 --view.lat 25', $f);
  },
 'HEIGHT'       , "Orography (raster & contour) with height                         " ,  contour_height      => '--field[0].path share/glgrib/testdata/t479/Z.grb  --field[0].scale 1.005 --field[0].type CONTOUR --field[1].geometry.height.on --field[0].palette.colors black --field[1].path share/glgrib/testdata/t479/Z.grb  --field[0].geometry.height.on  --view.lat -16 --view.lon 134',
 'HEIGHT'       , "Orography with height                                            " ,  orography_height    => '--field[0].path ./share/glgrib/testdata/t1798/Z.grb --field[0].palette.name topo --field[0].scale 1.005 --landscape.on --field[0].geometry.height.on',
 'HEIGHT'       , "Height = vector norm value                                       " ,  height_vector       => '--field[0].type VECTOR --field[0].path share/glgrib/testdata/wind+temp/t0224/S105WIND.U.PHYS.grb share/glgrib/testdata/wind+temp/t0224/S105WIND.V.PHYS.grb  --field[0].geometry.height.on --field[0].geometry.height.scale 0.2 --field[0].vector.arrow.color red --field[0].palette.name summer',
 'HEIGHT'       , "Height = scalar norm value                                       " ,  height_scalar       => '--field[0].path share/glgrib/testdata/wind+temp/t0224/S105TEMPERATURE.grb  --field[0].geometry.height.on --field[0].geometry.height.scale 0.2 --view.fov 24',
 'GRID'         , "Grid labels                                                      " ,  grid_labels         => '--grid.on --grid.labels.on --landscape.on', 
 'GRID'         , "Display low & high                                               " ,  hilo                => '--field[0].path share/glgrib/testdata/wind+temp/t0224/S105TEMPERATURE.grb --field[0].hilo.on --field[0].hilo.font.color.foreground green',
 'CONTOUR'      , "Contour shading                                                  " ,  contour5            => "--field[0].path share/glgrib/testdata/t850.grb  --field[0].palette-{ --colors '#0000ff' '#0055ff' '#00aaff' '#00ffff' '#00ffaa' '#00ff55' '#00ff55'  '#55ff00' '#aaff00' '#ffff00' '#ffaa00' '#ff5500' '#ff0000' '#dd0000' --values 253.15 263.15 268.15 270.65 272.15 272.65 273.15 273.65 274.15 275.65 278.15 283.15 293.15 303.15 350 }- --coast.on --coast.lines.color black --view.projection LATLON --view.lat 46.7 --view.lon 15 --view.fov 4 --field[1].path  share/glgrib/testdata/t850.grb --field[1].type contour --field[1].palette.colors '#95a3c0' --colorbar.on  --colorbar.font.color.foreground black --field[1].contour.levels 253.15 263.15 268.15 270.65 272.15 272.65 273.15 273.65 274.15 275.65 278.15 283.15 293.15 303.15  --field[0].palette.linear.on",
 'VECTOR'       , "Wind & vorticity                                                 " ,  wind4               => "--field[0].path share/glgrib/testdata/wind4/vorticity.grib --field[0].palette.colors '#00004cff' '#00007fff' '#0000b2ff' '#0000e5ff' '#0026ffff' '#004cffff' '#0072ffff' '#0099ffff' '#00bfffff' '#00d800ff' '#33f2ffff' '#72ffffff' '#bfffffff' '#ffffffff' '#ffff00ff' '#ffe500ff' '#ffcc00ff' '#ffb200ff' '#ff9900ff' '#ff7f00ff' '#ff6600ff' '#ff4c00ff' '#ff2600ff' '#e50000ff' '#b20000ff' '#7f0000ff' '#4c0000ff' --field[0].palette.values -0.002 -0.001 -0.00075 -0.0005 -0.0003 -0.0002 -0.00015 -0.00013 -0.00011 -9e-05 -7e-05 -5e-05 -3e-05 -1e-05 1e-05 3e-05 5e-05 7e-05 9e-05 0.00011 0.00013 0.00015 0.0002 0.0003 0.0005 0.00075 0.001 0.002 --field[0].palette.linear.on --field[0].palette.scale 100000 --field[1].scale 1.001 --field[1].path 'share/glgrib/testdata/wind4/wind.grib%shortName=\"u\"' 'share/glgrib/testdata/wind4/wind.grib%shortName=\"v\"' --field[1].type vector --field[1].vector.norm.off --field[1].vector.arrow.color black --colorbar.on --coast.on --coast.lines.color black --coast.on --coast.lines.color black --grid.on --grid.color black --view.lat 50 --view.fov 3 --view.projection POLAR_NORTH --view.clip.xmin 0.15 --render.width 1500 --grid.resolution 18",
 'MPI'          , "Display sphere partitionning (t1198)                             " ,  myproc              => "--field[0].scalar.discrete.on --field[0].path share/glgrib/testdata/discrete/MYPROC.grb --field[0].palette.colors green  --field[0].scalar.discrete.missing_color black --field[0].palette.colors '#00000000'  --field[1].path share/glgrib/testdata/discrete/SURFTEMPERATURE.grb --field[1]-{ --palette.max '313.15' --palette.min '253.15' --palette.name 'cold_hot_temp' }- --view.lat 36 --view.lon -15",
 'MPI'          , "Display sphere partitionning (t1192c2.2)                         " ,  myproc22            => "--field[0].scalar.discrete.on --field[0].path share/glgrib/testdata/discrete_stretched/MYPROC.grb --field[0].palette.colors green  --field[0].scalar.discrete.missing_color black --field[0].palette.colors '#00000000'  --field[1].path share/glgrib/testdata/discrete_stretched/SURFTEMPERATURE.grb  --field[1]-{ --palette.max '313.15' --palette.min '253.15' --palette.name 'cold_hot_temp' }-",
 'SCALAR'       , "SST on lat/lon grid                                              " ,  sst                 => '--field[0].scale 1.001 --field[0].path share/glgrib/testdata/sst.grib --field[0].palette-{ --values 271.15 273.15 275.15 277.15 279.15 281.15 283.15 285.15 287.15 289.15 291.15 293.15 295.15 297.15 299.15 301.15 303.15 305.15 307.15 309.15 --colors "#4a007aff" "#9c00ffff" "#cc78ffff" "#0000ffff" "#0059ffff" "#008cffff" "#0a7d00ff" "#0abf00ff" "#0aff00ff" "#a19c00ff" "#dad300ff" "#ffed00ff" "#a85400ff" "#d66b00ff" "#ff8500ff" "#bf0a00ff" "#ff0d00ff" "#ff857fff" "#ffd6d4ff" --offset -273.15  }-  --colorbar.on  --grid.on  --landscape.on --landscape.path landscape/black.png  --view.lon -150 --view.projection LATLON --render.width 1500  --view.fov 13 --grid.color gray',
 'CONTOUR'      , "2m temperature                                                   " ,  contour9            => '--field[0]-{ --path share/glgrib/testdata/2m_temperature.grib --palette.values 241.15 243.15 245.15 247.15 249.15 251.15 253.15 255.15 257.15 259.15 261.15 263.15 265.15 267.15 269.15 271.15 273.15 275.15 277.15 279.15 281.15 283.15 285.15 287.15 289.15 291.15 293.15 295.15 297.15 299.15 301.15 303.15 305.15 307.15 309.15 311.15 313.15 315.15 --palette.colors "#4c4c4cff" "#666666ff" "#7f7f7fff" "#999999ff" "#b2b2b2ff" "#ccccccff" "#590099ff" "#7f00e5ff" "#9933ffff" "#bf66ffff" "#d899ffff" "#0000bfff" "#0000ffff" "#3366ffff" "#66b2ffff" "#99e5ffff" "#008c30ff" "#26bf19ff" "#7fd800ff" "#a5f200ff" "#ccff33ff" "#a5a500ff" "#cccc00ff" "#eaea00ff" "#ffff00ff" "#ffff99ff" "#d87200ff" "#ff7f00ff" "#ff9e00ff" "#ffbc00ff" "#ffd800ff" "#990000ff" "#cc0000ff" "#ff0000ff" "#ff6666ff" "#ff9999ff" "#ffbfbfff" --palette.offset -273.15  --palette.linear.on }-  --colorbar.on  --coast.on --coast.lines.color black  --view.projection LATLON  --render.width 1500  --view.lon 10 --view.lat 52 --view.fov 3 --view.clip.xmin 0.15',
 'LAND'         , "Display land surface                                             " ,  land                => '--land.on',
 'LAND'         , "Display land surface (high resolution)                           " ,  land_high           => '--land.on --land.layers[0].path share/glgrib/coastlines/shp/GSHHS_h_L1.shp --land.layers[1].path share/glgrib/coastlines/shp/GSHHS_h_L2.shp --land.layers[2].path share/glgrib/coastlines/shp/GSHHS_h_L3.shp --land.layers[3].path share/glgrib/coastlines/shp/GSHHS_h_L5.shp  --view.lon 7 --view.lat 60 --view.fov 2',
 'MPI'          , "MPI view                                                         " ,  mpiview             => '--field[0].path share/glgrib/testdata/discrete/SURFTEMPERATURE.grb  --field[0].mpiview.on  --field[0].mpiview.path share/glgrib/testdata/discrete/MYPROC.grb  --field[0].mpiview.scale 0.2 --view.lon 31 --view.lat 41',
 'MPI'          , "MPI halo                                                         " ,  mpihalo             =>  sub
 {
   my $nproc = 80;
   my $i = shift;
   my $n = 10;
   my $p = $nproc / $n;
   return unless ($i < $n);
   use feature 'state';
   state @nn = do { my @n = (0 .. $nproc-1); my @x = map { rand () } @n; sort { $x[$a] <=> $x[$b] } @n };
   my @ii = @nn[($i*$p .. $i*$p+$p-1)];
   sprintf ('--field[0].scalar.widen.on --field[0].scalar.widen.values %s --field[0].scalar.widen.radius 20 '
          . '--field[0].scalar.discrete.on --field[0].path share/glgrib/testdata/discrete/MYPROC.grb '
          . '--field[0].palette.colors green --field[0].scalar.discrete.missing_color black '
          . '--field[0].palette.colors #00000000 --field[1].path share/glgrib/testdata/discrete/SURFTEMPERATURE.grb '
          . '--field[1]-{ --palette.max 313.15 --palette.min 253.15 --palette.name cold_hot_temp }- '
          . '--view.lat 36 --view.lon -15', join (' ', map { sprintf ('%d.0', $_ + 1) } @ii));
 },
 'MPI'          , "MPI transpositions                                               " ,  mpitrans            => sub
 {
   my $n = 4;
   my $i = shift;
   return unless ($i < 6 * $n);

   my @x = ('', '.NPRGPEW=1', '.NPRGPNS=1', '.NPRGPNS=1', '.NPRGPEW=1', '');
   @x = map { ($_) x $n } @x;

   my $myproc = 'share/glgrib/testdata/discrete/MYPROC' . $x[$i] . '.grb';

   sprintf ('--field[0].scalar.discrete.on --field[0].path %s --field[0].scalar.discrete.missing_color black '
          . '--field[0].palette.colors #00000000 --field[1].path share/glgrib/testdata/discrete/SURFTEMPERATURE.grb '
          . '--field[1]-{ --palette.max 313.15 --palette.min 253.15 --palette.name cold_hot_temp }- --view.lat 36 --view.lon -15', $myproc);

 },
 'MPI'          , "MPI halo                                                         " ,  mpihalo1            => '--field[0].scalar.discrete.on --field[0].path share/glgrib/testdata/discrete/MYPROC.grb --field[0].scalar.discrete.missing_color black --field[0].palette.colors #00000000 --field[1].scalar.widen.on --field[1].scalar.widen.merge.on --field[1].scalar.widen.merge.value -1 --field[1].scalar.widen.values 0 2 3 75 68 29 70 32 28 73 51 30 --field[1].scalar.widen.radius 20 --field[1].scalar.discrete.on --field[1].path share/glgrib/testdata/discrete/MYPROC.grb --field[1].scalar.discrete.missing_color #00aa00ff --field[1].palette.colors #00000000 --field[1].scale 1.001 --field[2].path share/glgrib/testdata/discrete/SURFTEMPERATURE.grb --field[2]-{ --palette.max 313.15 --palette.min 253.15 --palette.name cold_hot_temp }- --view.lat 36 --view.lon -15',
 'MPI'          , "MPI halo                                                         " ,  mpihalo2            => '--field[0].path share/glgrib/testdata/discrete/SURFTEMPERATURE.grb --field[0]-{ --palette.max 313.15 --palette.min 253.15 --palette.name cold_hot_temp }- --field[1].scalar.widen.on --field[1].scalar.widen.merge.on --field[1].scalar.widen.values 0 2 3 75 68 29 70 32 28 73 51 30 --field[1].scalar.widen.radius 20 --field[1].path share/glgrib/testdata/discrete/MYPROC.grb --field[1].palette.colors #00000000 #00ff0055 --field[1].palette.values 0 1 80 --field[1].scale 1.02 --field[2].scalar.discrete.on --field[2].path share/glgrib/testdata/discrete/MYPROC.grb --field[2].scalar.discrete.missing_color black --field[2].palette.colors #00000000 --field[2].scale 1.01 --view.lat 36 --view.lon -15',
 'MPI'          , "MPI halo                                                         " ,  mpihalo3            => '--field[0].path share/glgrib/testdata/t1798c2.2/SFX.HU2M.grb --field[1].scalar.discrete.on --field[1].path share/glgrib/testdata/discrete/MYPROC.grb --field[1].scalar.discrete.missing_color black --field[1].palette.colors #00000000 --field[1].scale 1.001 --field[2].palette.colors #00000000 #ff000055 --field[2].palette.values 0 1 80 --field[2].path share/glgrib/testdata/discrete/MYPROC.grb --field[2].scalar.discrete.missing_color black --field[2].scalar.discrete.on --field[2].scalar.widen.merge.on --field[2].scalar.widen.merge.value -1 --field[2].scalar.widen.on --field[2].scalar.widen.radius 20 --field[2].scalar.widen.values 0 2 3 75 68 29 70 32 28 73 51 30 --field[2].scale 1.002 --view.lat 36 --view.lon -15 --colorbar.on --render.width 1200',
 'STREAM'       , "Lat/lon streamlines                                              " ,  latlonstream        => '--field[0].path share/glgrib/testdata/advection_850.grib%\'shortName="u"\' share/glgrib/testdata/advection_850.grib%\'shortName="v"\' --field[0].type STREAM --field[0].palette.colors darkblue --land.on --land.layers[0].path share/glgrib/coastlines/shp/GSHHS_i_L1.shp --land.layers[1].path share/glgrib/coastlines/shp/GSHHS_i_L2.shp --land.layers[2].path share/glgrib/coastlines/shp/GSHHS_i_L3.shp --land.layers[3].path share/glgrib/coastlines/shp/GSHHS_i_L5.shp --land.layers[0].color grey --grid.on --grid.resolution 18  --grid.color black  --landscape.on --landscape.path landscape/white.bmp --landscape.scale 0.99 --view.lon -9.5 --view.lat 46 --view.fov 1.3 --view.projection LATLON  --render.width 1200',
 'VECTOR'       , "Wind on global lat/lon grid                                      " ,  windlatlon          => '--field[0].path share/glgrib/testdata/data_uv.grib%shortName=\'"u"\' share/glgrib/testdata/data_uv.grib%shortName=\'"v"\' --field[0].type vector --field[0].vector.arrow.off  --field[0].palette.values 0 10 15 20 25 30 40 50 60 80 100  --colorbar.on  --field[0].palette.linear.on --field[0].palette-{ --min 0 --max 100 }- --field[0].palette.colors \'#ffffffff\' \'#ffff66ff\'  \'#daff00ff\' \'#94ff00ff\' \'#6ca631ff\' \'#00734bff\' \'#005447ff\' \'#004247ff\' \'#003370ff\' \'#0033a3ff\'  --coast.on --coast.lines.color black  --grid.on --grid.color black --grid.resolution 18 --view.fov 10 --view.projection LATLON --render.width 1650 --render.height 750',
 'CONTOUR'      , "Z500, T850                                                       " ,  z500t850            => '--coast.lines.color black --coast.on --colorbar.font.color.foreground black --colorbar.on --field[0].palette.colors \'#ffffffff\' \'#0000ffff\' \'#0071ffff\' \'#00e3ffff\' \'#00ffaaff\' \'#00ff39ff\' \'#39ff00ff\' \'#aaff00ff\' \'#ffe300ff\' \'#ff7100ff\' \'#ff0000ff\' \'#ffffffff\' --field[0].palette.offset -273.15 --field[0].palette.values 262.15 263.15 265.15 267.15 269.15 271.15 273.15 275.15 277.15 279.15 281.15 283.15 284.15 --field[0].path share/glgrib/testdata/t850.grb --field[1].contour.levels  47000 47500 48000 48500 49000 49500 50000 50500 51000 51500 52000 52500 53000 53500 54000 54500 55000 55500 56000 56500 57000 57500 --field[1].contour.widths 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 1 0 0 0 0 0 0  --field[1].hilo.font.color.foreground black --field[1].hilo.font.bitmap.scale 0.02  --field[1].hilo.on  --field[1].palette.colors black  --field[1].path share/glgrib/testdata/z500.grb --field[1].type CONTOUR  --grid.color black --grid.labels.font.color.background white --grid.labels.font.color.foreground  black  --grid.labels.font.bitmap.scale 0.02 --grid.labels.on  --grid.on --grid.resolution 18 --view.fov 5 --view.lat 49 --view.lon 2 --view.projection POLAR_NORTH --render.width 1200',
 'CONTOUR'      , "Joachim storm                                                    " ,  joachim             => '--field[0].scale 1.0005 --field[0].path \'share/glgrib/testdata/joachim_surf.grib%stepRange="18-24",shortName="10fg6"\' --field[0].palette.values 0 20 20.6 21.2 21.8 22.4 23 23.6 24.2 24.8 25.4 26 27.5 29 30.5 32 33.5 35 36.5 38 --field[0].palette.colors \'#00000000\' \'#88a8d6ff\' \'#819cd2ff\' \'#7a8fceff\' \'#7382caff\' \'#6c74c6ff\' \'#6666c2ff\' \'#6760bdff\' \'#6959b8ff\' \'#6b53b4ff\' \'#6e4fadff\' \'#9045b8ff\' \'#bf3bc2ff\' \'#cc319dff\' \'#d62869ff\' \'#e11e28ff\' \'#eb4e14ff\' \'#f5960aff\' \'#ffea01ff\' --field[0].palette.linear.on --colorbar.on --field[1].path \'share/glgrib/testdata/joachim_surf.grib%stepRange=24,shortName="10u"\' \'share/glgrib/testdata/joachim_surf.grib%stepRange=24,shortName="10v"\' --field[1].scale 1.001 --field[1].type vector --field[1].vector.norm.off --field[1].vector.arrow.color black --field[1].vector.density 20 --field[2].path \'share/glgrib/testdata/joachim_surf.grib%stepRange=24,shortName="msl"\' --field[2].scale 1.001 --field[2].type contour --field[2].palette.colors black --field[2].contour.levels 97000 97500 98000 98500 99000 99500 100000 100500 101000 101500 102000 --field[2].contour.widths 3 3 3 3 1 1 1 1 1 1 3 --render.width 1200 --grid.on --grid.color black --grid.resolution 18 --coast.on --coast.lines.color black --coast.lines.path share/glgrib/coastlines/gshhg/GSHHS_bin/gshhs_i.b --border.on --border.lines.path share/glgrib/coastlines/gshhg/WDBII_bin/wdb_borders_i.b --view.clip.xmin 0.15 --view.projection POLAR_NORTH --land.on --land.layers[0].color \'#7c879cff\' --land.layers[0].path share/glgrib/coastlines/shp/GSHHS_i_L1.shp --land.layers[1].path share/glgrib/coastlines/shp/GSHHS_i_L2.shp --land.layers[2].path share/glgrib/coastlines/shp/GSHHS_i_L3.shp --land.layers[3].path share/glgrib/coastlines/shp/GSHHS_i_L5.shp --landscape.on --landscape.path "" --landscape.color white --landscape.scale 0.999 --view.lon 4 --view.lat 51 --view.fov 3',
 'ISOFILL'      , "Isofill test                                                     " ,  isofill             => '--field[0].path share/glgrib/testdata/wind+temp/t1798/S105TEMPERATURE.grb --field[0].type ISOFILL --field[0]-{ --palette.min 294 --palette.max 296 --user_pref.off --isofill.min 294 --isofill.max 296 --palette.name cold_hot_temp }- --view.lat 2.32 --view.lon 14.12 --view.fov 0.5 --render.width 1200',
 'ISOFILL'      , "Isofill test wireframe                                           " ,  isofillwire         => '--field[0].path share/glgrib/testdata/wind+temp/t1798/S105TEMPERATURE.grb --field[0].type ISOFILL --field[0]-{ --scalar.wireframe.on --palette.min 294 --palette.max 296 --user_pref.off --isofill.min 294 --isofill.max 296 --palette.name cold_hot_temp }- --view.lat 2.32 --view.lon 14.12 --view.fov 0.5 --render.width 1200',
 'ISOFILL'      , "Isofill temperature                                              " ,  isofilltemp         => '--field[0].path share/glgrib/testdata/wind+temp/t1798/S105TEMPERATURE.grb --field[0].type ISOFILL --field[0]-{ --palette.name cold_hot_temp }-  --colorbar.on  --render.width 1200 --coast.on --coast.lines.color black',
 'ISOFILL'      , "Isofill Lambert                                                  " ,  isofilllambert      => '--landscape.on --field[0].type ISOFILL --field[0].user_pref.off --field[0].path share/glgrib/testdata/aro_guyane/SURFTEMPERATURE.grb --field[0].palette.name cold_hot_temp --field[0].scale 1.01 --view.lat 5 --view.lon -52 --view.fov 2 --coast.on --grid.on --colorbar.on  --render.width 1200 --coast.lines.scale 1.02 --coast.lines.color black',
 'ISOFILL'      , "Isofill latlon                                                   " ,  isofilllatlon       => '--landscape.on --field[0].type ISOFILL --field[0].path share/glgrib/testdata/eurat01/lfpw_0_0_0_pl_1000_t.grib2 --field[0].scale 1.01 --view.lat 46.7 --view.lon 2 --view.fov 10 --coast.on --grid.on --colorbar.on  --render.width 1200 --coast.lines.scale 1.02 --coast.lines.color black',
 'SCALAR'       , "Old ARPEGE                                                       " ,  oldarpege           => '--field[0].path share/glgrib/testdata/arpege.123.fa%SURFTEMPERATURE --scene.date.on --scene.date.font.bitmap.scale 0.03',
 'STRING'       , "Contour with labels (dummy field)                                " ,  contourlabels       => '--landscape.on --field[0].path share/glgrib/testdata/contour/latlon18x13.grb --field[0].scale 1.03 --field[0].type CONTOUR --view.lat 45.65 --view.lon 6.39 --field[0].contour.labels.on --grid.on --view.fov 10  --grid.color white --field[0].palette.colors red  --field[0].contour.labels-{ --font.color.foreground white --font.color.background black }- --coast.on --coast.lines.color pink',
 'POINTS'       , "Polynesie                                                        " ,  polynesie           => '--field[0].path share/glgrib/testdata/polynesie/SURFTEMPERATURE.grb --view.lat -17.63 --view.lon -149.5 --view.fov 0.2 --coast.lines.color green --coast.on  --field[0].scalar.points.on --colorbar.on  --render.width 1200  --colorbar.font.color.background black',
 'VECTOR'       , "Wind Australia                                                   " ,  windaustralia       => '--field[0].path share/glgrib/testdata/uv200.grib%paramId=131 share/glgrib/testdata/uv200.grib%paramId=132 --field[0].type VECTOR --coast.on  --view.lat -25 --view.lon 140 --view.fov 8 --field[0].vector.norm.off --land.on  --field[0].vector.arrow.color blue --landscape.on --landscape.color white --landscape.scale 0.999 --coast.on --coast.lines.color black --coast.lines.path "share/glgrib/coastlines/gshhg/GSHHS_bin/gshhs_i.b"  --field[0].vector.density 100 --field[0].vector.scale 10  --render.width 1200 --field[0].scale 1.001',
 'VECTOR'       , "Wind Australia                                                   " ,  windaustralia2      => '--field[0].path share/glgrib/testdata/uv200.grib%paramId=131 share/glgrib/testdata/uv200.grib%paramId=132 --field[0].type VECTOR --coast.on  --view.lat -25 --view.lon 140 --view.fov 2 --field[0].vector.norm.off --land.on  --field[0].vector.arrow.color darkgreen --landscape.on --landscape.color white --landscape.scale 0.999 --coast.on --coast.lines.color black --coast.lines.path "share/glgrib/coastlines/gshhg/GSHHS_bin/gshhs_i.b"  --field[0].vector.scale 5  --render.width 1200 --field[0].scale 1.001 --view.projection LATLON  --field[0].vector.density 10',
 'CONTOUR'      , "Precipitations Europe                                            " ,  precipeurope        => '--field[0].path share/glgrib/testdata/total_precipitation.grib  --coast.on --coast.lines.color black --landscape.color white --landscape.scale 0.999 --landscape.on --field[0].palette.colors "#00000000" "#00ffff" "#0080ff" "#0000ff" "#da00ff" "#ff00ff" "#ff8000" "#ff0000" --field[0].palette.values 0. 0.0005 0.002 0.004 0.010 0.025 0.050 0.100 0.250  --colorbar.on   --field[0].palette.linear.on --field[0].type ISOFILL  --render.width 1200  --field[0].isofill.levels 0. 0.0005 0.002 0.004 0.010 0.025 0.050 0.100 0.250 --field[1].type CONTOUR --field[1].path share/glgrib/testdata/total_precipitation.grib --field[1].scale 1.001 --field[1].contour.levels 0. 0.0005 0.002 0.004 0.010 0.025 0.050 0.100 0.250 --field[1].palette.colors black  --colorbar.font.color.foreground white  --view.lon 2 --view.lat 46.2 --view.fov 5 --view.projection POLAR_NORTH --grid.on --grid.color black --grid.labels.on  --grid.labels.font.color.foreground black  --grid.labels.font.bitmap.scale 0.03 --grid.resolution 18 --field[0].palette.scale 1000 --colorbar.font.color.background black --view.clip.xmin 0.15',
 'CONTOUR'      , "Precipitations Europe (precise, with fixed palette)              ",   precipfixed         => '--field[0].path share/glgrib/testdata/total_precipitation.grib --field[0].palette.colors "#00000000" "#bbffff" "#00ffff" "#0080ff" "#0000ff" "#da00ff" "#ff00ff" "#ff8000" "#ff0000" --field[0].palette.values 0. 0.0001 0.0005 0.002 0.004 0.010 0.025 0.050 0.100 0.250 --field[0].palette.linear.on --field[0].type ISOFILL --field[0].palette.fixed.on --field[0].isofill.levels 0. 0.0001 0.0005 0.002 0.004 0.010 0.025 0.050 0.100 0.250 --coast.on --coast.lines.color black --landscape.color white --landscape.scale 0.999 --landscape.on --render.width 1200 --grid.labels.on --grid.labels.font.color.foreground black --grid.labels.font.bitmap.scale 0.03 --grid.resolution 18 --field[0].palette.scale 1000 --view.lon 2 --view.lat 46.2 --view.fov 5 --view.projection POLAR_NORTH --grid.on --grid.color black --colorbar.on --colorbar.font.color.background black --view.clip.xmin 0.15 --colorbar.font.color.foreground white --field[1].type CONTOUR --field[1].path share/glgrib/testdata/total_precipitation.grib --field[1].scale 1.001 --field[1].contour.levels 0. 0.0001 0.0005 0.002 0.004 0.010 0.025 0.050 0.100 0.250 --field[1].palette.colors black',
 'CONTOUR'      , "Z500                                                             " ,  z500europe          => '--field[0]-{ --contour.labels.format "%5.0f" --contour.widths 1 2 1 1 2 1 1 2 1 1 2 --path share/glgrib/testdata/z500.grb --type CONTOUR --palette.colors black --contour.labels.on --contour.labels.font.color.foreground black --contour.labels.font.color.background white --contour.levels 51000 51600 52200 52800 53400 54000 54600 55200 55800 56400 57000  }-  --landscape.on --landscape.scale 0.999 --landscape.color white --coast.on --coast.lines.color black  --view.lat 46.2 --view.lon 2 --view.fov 5 --grid.on --grid.color black --grid.resolution 18 --view.projection POLAR_NORTH',
 'CONTOUR'      , "Temperature, contour, colors                                     " ,  tempcontour         => '--field[0].scale 1.001 --field[0].path share/glgrib/testdata/t850.grib --field[0].palette.colors "#0000ff" "#003cff" "#0078ff" "#00b4ff" "#00f0ff" "#00ffd2" "#00ff96" "#00ff5a" "#00ff1e" "#1eff00" "#5aff00" "#96ff00" "#d2ff00" "#fff000" "#ffb400" "#ff7800" "#ff3c00" "#ff0000" --field[0].palette.values  259.15 261.15 263.15 265.15 267.15 269.15 271.15 273.15 275.15 277.15 279.15 281.15 283.15 285.15 287.15 289.15  291.15 --colorbar.on --render.width 1200  --field[0].type CONTOUR  --field[0].contour.levels 259.15 261.15 263.15 265.15 267.15 269.15 271.15 273.15 275.15 277.15 279.15 281.15 283.15 285.15 287.15 289.15  --landscape.on --landscape.color black --landscape.scale 0.999 --land.on  --view.lon 2 --view.lat 46.2 --view.fov 4 --field[0].contour.widths 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 --land.layers[0].path share/glgrib/coastlines/shp/GSHHS_h_L1.shp  --land.layers[0].color "#404040"  --view.clip.xmin 0.15 --grid.color grey --grid.resolution 18 --grid.on',
 'VECTOR'       , "Wind Australia                                                   " ,  windaustralia4      => '--field[0].path share/glgrib/testdata/uv200.grib%paramId=131 share/glgrib/testdata/uv200.grib%paramId=132 --field[0].type VECTOR --coast.on --view.lat -25 --view.lon 140 --view.fov 8 --field[0].vector.norm.off --land.on --landscape.on --landscape.color white --landscape.scale 0.999 --coast.on --coast.lines.color black --coast.lines.path share/glgrib/coastlines/gshhg/GSHHS_bin/gshhs_i.b --field[0].vector.density 20 --field\[0\].vector.scale 0.5 --render.width 1200 --field\[0\].scale 1.001 --field[0].vector.arrow.off --field[0].vector.barb.on  --field[0].vector.barb.color brown  --field[0].vector.barb.circle.level 15',
 'VECTOR'       , "Wind + geopotential                                              " ,  windgeop            => '--field[0].palette.colors black --field[0].contour.widths 1 2 1 1 2 1 1 2 1 1 2 --field[0].contour.levels 89000 89500 90000 90500 91000 91500 92000 92500 93000 93500 94000 94500 95000 95500 96000 96500 97000 97500 98000 98500 99000 99500  --field[0].path share/glgrib/testdata/ghtuv.grib%\'shortName="z"\' --field[0].type CONTOUR --land.on  --landscape.on --landscape.color white --landscape.scale 0.999  --field[1]-{ --type vector --path share/glgrib/testdata/ghtuv.grib%\'shortName="u"\' share/glgrib/testdata/ghtuv.grib%\'shortName="v"\' --vector-{ --norm.off --arrow.off --barb.on --barb.color blue --density 20 --scale 0.4 }- --scale 1.001 }-  --render.width 1200 --view.lat 46.2 --view.lon -10  --view.fov 5 --view.projection POLAR_NORTH  --grid.on  --grid.color black --grid.resolution 18',
 'SCALAR'       , "Simple shading                                                   " ,  shading             => '--field[0].path share/glgrib/testdata/t850.grb  --field[0].palette.values 245.15 297.15 --field[0].palette.generate.on --field[0].palette.generate.levels 53 --field[0].palette.colors blue red  --colorbar.on --render.width 1200  --field[0].palette.rainbow.on --field[0].palette.rainbow.direct.off --field[0].palette.offset -273.15 --coast.on  --coast.lines.color black  --grid.on --grid.color black  --grid.labels.on --grid.labels.font.color.foreground black --grid.labels.font.color.background white  --grid.labels.font.bitmap.scale 0.04 --render.width 1200 --view.lat 34 --view.lon 20',
 'LAND'         , "Coastlines                                                       " ,  coastlines          => '--landscape.on --landscape.scale 0.999  --land.on  --landscape.color white --rivers.on --rivers.lines.color blue --border.on --border.lines.color red --coast.on --coast.lines.color black  --coast.lines.path "share/glgrib/coastlines/gshhg/GSHHS_bin/gshhs_i.b" --land.layers[0].path share/glgrib/coastlines/shp/GSHHS_i_L1.shp --land.layers[1].path share/glgrib/coastlines/shp/GSHHS_i_L2.shp  --view.fov 5  --view.lat 44.5394 --view.lon    27.5924  --render.width 1200',
 'SCALAR'       , "Rain, southern America                                           " ,  southamerica        => '--field[0].path  share/glgrib/testdata/total_precipitation.grib --field[0].palette-{ --values 0. 0.0005 0.001 0.002 0.005 0.010 0.020 0.050 0.100 0.200 0.250 --colors white "#ffffd9" "#edf8b1" "#c7e9b4" "#7fcdbb" "#41b6c4" "#1d91c0" "#225ea8" "#253494" "#081d58" }-  --coast.on --coast.lines.color black   --field[0].palette.linear.on  1000.  --colorbar.on  --render.width 1200 --colorbar.font.color.foreground black  --view.projection LATLON   --view.fov 4  --view.lat  -20.1373  --view.lon  -60.4499  --grid.on --grid.color black',
 'GRID'         , "Grid ticks & labels                                              " ,  tickslabels         => '--landscape-{ --on --scale 0.999 }- --grid-{ --on --resolution 18 }-  --ticks-{ --labels.on --labels.font.bitmap.scale 0.02 --labels.format "%.0f" --lines.on --lines.color blue --lines.width 0.03 --lines.kind 1 }- --view.fov 7  --view.lat 50  --view.lon  0',
 'STREAM'       , "Stream motion                                                    " ,  streammotion        => '--field[0].stream.motion.on --field[0].path share/glgrib/testdata/advection_850.grib%\'shortName="u"\' share/glgrib/testdata/advection_850.grib%\'shortName="v"\' --field[0].type STREAM --field[0].palette.name cold_hot --land.on --grid.on --grid.resolution 18 --grid.color black --landscape.on --landscape.path landscape/white.bmp --landscape.scale 0.99 --view.lon -10.364 --view.lat 46.8244 --view.fov 5.3 --view.projection XYZ --render.width 1200 --field[0].stream.width 0.5 --field[0].scale 1.001 --ticks.labels.on --ticks.labels.font.color.foreground black --render.offscreen.on --render.offscreen.frames 36',
 'MISC'         , "Zoom with Schmidt transform                                      " ,  zoomschmidt1        => '--field[0].path share/glgrib/testdata/wind+temp/t1798/S105TEMPERATURE.grb --view.zoom.on  --coast.on --coast.lines.color black --view.lon 2 --view.lat 46.7 --view.zoom.stretch 0.2 --grid.off',
 'SCALAR'       , "Temperature with fixed palette                                   " ,  fixedpaltemp        => '--field[0].path share/glgrib/testdata/wind+temp/t0224/S105TEMPERATURE.grb --field[0].user_pref.off --field[0].palette-{ --colors blue pink orange yellow cyan --values 260 270 280 290 300 320 --fixed.on --linear.on }-  --field[0].type ISOFILL --field[0].isofill.levels 260 270 280 290 300 320 --colorbar.on  --render.width 1200 --coast.on --coast.lines.color black',
 'SCALAR'       , "Temperature with fixed palette (smooth)                          " ,  fixedpalscalsmooth  => '--field[0].path share/glgrib/testdata/wind+temp/t0224/S105TEMPERATURE.grb --field[0].user_pref.off --field[0].palette-{ --colors blue pink orange yellow cyan --values 260 270 280 290 300 320 --fixed.on --linear.on }-  --colorbar.on  --render.width 1200 --coast.on --coast.lines.color black  --field[0].scalar.smooth.on',
 'SCALAR'       , "Temperature with fixed palette (rough)                           " ,  fixedpalscalrough   => '--field[0].path share/glgrib/testdata/wind+temp/t0224/S105TEMPERATURE.grb --field[0].user_pref.off --field[0].palette-{ --colors blue pink orange yellow cyan --values 260 270 280 290 300 320 --fixed.on --linear.on }-  --colorbar.on  --render.width 1200 --coast.on --coast.lines.color black',
 'SCALAR'       , "Display field in offscreen mode city lights                      " ,  citylight           => '--landscape.on --render.offscreen.on --render.offscreen.frames 36 --scene.light.rotate.on --scene.light.on --field[0].path ./share/glgrib/testdata/town/town1800.grb  --field[0].palette-{ --colors "#bbbb0000" "#bbbb00ff" "#bbbb00ff"  --values 0. 0.1 1.0 }- --field[0].scalar.light.reverse.on --field[1].path share/glgrib/testdata/t1198c2.2/SURFNEBUL.BASSE.grb --field[1].scale 1.03 --field[1].palette.name cloud_auto --scene.light.rotate.rate 10',
 'SATELLITE'    , "SSMI satellite data                                              " ,  ssmigeopoints       => '--geopoints.path share/glgrib/testdata/geopoints/ssmis_light.nc --geopoints.on  --geopoints.points.scale 1.005 --geopoints.points.palette.name cold_hot  --landscape.on  --geopoints.points.size.value 0.5 --grid.on --grid.color black --coast.on --coast.lines.color black --colorbar.on  --geopoints.points.palette.min 200 --geopoints.points.palette.max 240 --geopoints.lon col_4 --geopoints.lat col_3 --geopoints.val col_5 --render.width 1200 --view.lat -32.736 --view.lon 41.909',
 'SATELLITE'    , "SSMI satellite data (polar projection)                           " ,  ssmigeopointspolar  => '--geopoints.path share/glgrib/testdata/geopoints/ssmis_light.nc --geopoints.on --geopoints.points.palette.name cold_hot --landscape.on --geopoints.points.size.value 0.5 --grid.on --grid.color black --coast.on --coast.lines.color black --colorbar.on --geopoints.points.palette.min 200 --geopoints.points.palette.max 240 --geopoints.lon col_4 --geopoints.lat col_3 --geopoints.val col_5 --render.width 1200 --view.lat 62.577 --view.lon 71.4523 --view.projection POLAR_NORTH',
 'SATELLITE'    , "SSMI satellite data (lat/lon)                                    " ,  ssmigeopointlatlon  => '--geopoints.path share/glgrib/testdata/geopoints/ssmis_light.nc --geopoints.on --geopoints.points.palette.name cold_hot --landscape.on --geopoints.points.size.value 0.5 --grid.on --grid.color black --coast.on --coast.lines.color black --colorbar.on --geopoints.points.palette.min 200 --geopoints.points.palette.max 240 --geopoints.lon col_4 --geopoints.lat col_3 --geopoints.val col_5 --render.width 1200  --view.fov 10 --view.lat 5.04296 --view.lon -9.39295 --view.projection LATLON  --view.clip.xmin 0.15',
);


my @t = @test;
@test = ();
while (my ($kind, $desc, $name, $opts) = splice (@t, 0, 4))
  {
    $desc =~ s/\s*$//o;
    my @opts;
    if (ref ($opts))
      {
        my $i = 0;
        while (my $o = $opts->($i++))
          {
            $o =~ s/\n/ /goms;
            push @opts, [&quotewords ('\s+', 0, $o)];
          }
      }
    else
      {
        $opts =~ s/\n/ /goms;
        @opts = ([&quotewords ('\s+', 0, $opts)]);
      }
    $test{$name} && die ("Duplicate test: $name\n");
    $test{$name} = [$desc, \@opts];
    push @test, ($kind, $desc, $name, \@opts);
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
    
    my $exec = "$Bin/../bin/glgrib";
    
    my @argl = @{ $test->[1] };

    if ($argl[0][0] !~ m/^--/o)
      {
        $auto = 0;
      }
    
    if ($auto)
      {
        for my $argl (@argl)
          {
            my $off = grep { m/^--render.offscreen.on$/o } @$argl;
            push (@$argl, '--render.offscreen.on') unless ($off);
            push (@$argl, '--render.offscreen.format', 'TEST_%N.png');
            for (<TEST*.png>)
              {
                unlink ($_);
              }
            @$argl = grep { $_ ne '--shell.on' } @$argl;
          }
      }
    
    for my $argl (@argl)
      {
        
        my @cmd = ($exec, @$argl);
        @cmd = ('gdb', '-ex=set confirm on', '-ex=run', '-ex=quit', '--args', @cmd) if ($ENV{GDB});
        
        print "Running test $name...\n"; 
        print "@cmd\n";
     
        system (@cmd)
          and die;
      }
    

    if ($auto)
      {
        &mkpath ("test.run/$name");
        
	my @img = <TEST*.png>;

	if ((scalar (@img) > 1) && (! $ENV{NOMOVIE}))
	  {
            system ('convert', -delay => 20, -loop => 0, @img, 'TEST.gif');
	    rename ('TEST.gif', "test.run/$name/TEST.gif");
	  }

        for my $png (@img)
          {
            my $new = "test.run/$name/$png";
            my $thumb = "test.run/$name/thumb_$png";
            rename ($png, $new);
#           system ('convert', -geometry => 300, $new, $thumb);
            my $ref = "share/glgrib/doc/samples/$name/$png";
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
        
