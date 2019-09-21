#!/usr/bin/perl -w
#

use strict;
use WWW::Mechanize;
use HTTP::Request::Common;
use FileHandle;
use Math::Trig;
use Data::Dumper;
use DB_File;
use Fcntl qw (O_RDWR O_CREAT);

if (@ARGV != 5)
  {
    die "Usage: $0 width, lon1, lat1, lon2, lat2\n";
  }

my ($width, $lon1, $lat1, $lon2, $lat2) = @ARGV;

my $key = 'an7nvfzojv5wa96dsga5nk8w';


my ($db, %webmercator);

if (-f 'webmercator.db')
  {
    $db = tie (%webmercator, 'DB_File', 'webmercator.db', O_RDWR, 0644, $DB_BTREE);
  }
else
  {
    $db = tie (%webmercator, 'DB_File', 'webmercator.db', O_RDWR | O_CREAT, 0644, $DB_BTREE);
  }

my $deg2rad = pi / 180.;
my $rad2deg = 180. / pi;

my $Ra = 6378137;


while ($lon2 < $lon1)
  {
    $lon2 += 360;
  }

if ($lat2 > $lat1)
  {
    ($lat1, $lat2) = ($lat2, $lat1);
  }

$width = $width * 360 / ($lon2 - $lon1);

my $lev = 1 + int (log ($width/256) / log (2));

for ($lon1, $lat1, $lon2, $lat2)
  {
    $_ = $_ * $deg2rad;
  }

sub lonlat2XY 
{
  my ($lon, $lat) = @_;
  my $X = $Ra * $lon;
  my $Y = $Ra * log (tan (pi / 4. + $lat * 0.5));
  return ($X, $Y);
}

sub lonlat2IXIY
{
  my ($X, $Y) = &lonlat2XY (@_);

  my $X0 = -20037508.3427892476320267;
  my $Y0 = +20037508.3427892476320267;

  my $F = 2 * pi * $Ra / (256*2**$lev);

  my $Z = 256 * $F;

  my $IX = int (($X - $X0) / $Z); # = 132877.090037192
  my $IY = int (($Y0 - $Y) / $Z); # =  90241.351534632

  return ($IX, $IY);
}


my $ua ='WWW::Mechanize'->new (autocheck => 0);
$ua->agent_alias ('Linux Mozilla');



my ($IX1, $IY1) = &lonlat2IXIY ($lon1, $lat1);
my ($IX2, $IY2) = &lonlat2IXIY ($lon2, $lat2);


# https://wxs.ign.fr/ld0jgrlpaway88fw6u4x3h38/geoportail/wmts?layer=ORTHOIMAGERY.ORTHOPHOTOS&style=normal
# &tilematrixset=PM&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg
# &TileMatrix=14&TileCol=8180&TileRow=5905


my $colN = 2**$lev;

my ($rowA, $rowB) = ($IY1, $IY2);
my ($colA, $colB) = ($IX1, $IX2);

if ($colB - $colA + 1 > $colN)
  {
    $colB = $colA + $colN - 1;
  }



my @imgY;
for my $row ($rowA .. $rowB)
  {
    my @imgX;
    my $r = $row;
    for my $col ($colA .. $colB)
      {
        my $c = $col % $colN;
        my $url = "https://wxs.ign.fr/$key/geoportail/wmts?layer=ORTHOIMAGERY.ORTHOPHOTOS&"
                . 'style=normal&tilematrixset=PM&Service=WMTS&Request=GetTile&Version=1.0.0&Format=image%2Fjpeg&'
        	. "TileMatrix=$lev&TileCol=$c&TileRow=$r";
        
        
        my $file = sprintf ('Tile_%5.5d_%5.5d_%5.5d.jpg', $lev, $c, $r);
	if (exists $webmercator{$file})
	  {
            'FileHandle'->new (">$file")->print ($webmercator{$file});
	  }
	else
	  {
            print "$file\n";
            my $rp = $ua->request (GET $url);
	    if ($rp->is_success ())
	      {
                'FileHandle'->new (">$file")->print ($rp->content ());
		$webmercator{$file} = do { my $fh = 'FileHandle'->new ("<$file"); local $/ = undef; <$fh> };
	      }
	    else
	      {
                die;
	      }
	  }
    
	push @imgX, $file;
      }

    my $imgY = sprintf ('Row_%5.5d.jpg', $row);

    my $n = scalar (@imgX);
    system ('montage', -tile => "${n}x1", -geometry => '+0+0', @imgX, $imgY)
      and die ("Cannot montage $imgY");


    unshift @imgY, $imgY;

    unlink ($_) for (@imgX);

  }

my $imgA = sprintf ('WebMercator_%5.5d_%5.5d_%5.5d_%5.5d_%5.5d.png', $lev, $rowA, $colA, $rowB, $colB);


my $n = scalar (@imgY);
system ('montage', -tile => "1x${n}", -geometry => '+0+0', reverse (@imgY), $imgA)
  and die ("Cannot montage $imgA");

unlink ($_) for (@imgY);

(my $bmp = $imgA) =~ s/\.png$/.bmp/go;

system ('convert', $imgA, $bmp);


