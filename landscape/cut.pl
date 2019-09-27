#!/usr/bin/perl -w
#
use strict;
use FileHandle;
use Data::Dumper;
use Image::Magick;


my $f = shift;

my $p = 'Image::Magick'->new ();

$p->Read ($f);

my ($w, $h) = ($p->Get ('width'), $p->Get ('height'));

my ($lon1a, $lat1a, $lon2a, $lat2a, $lon1b, $lat1b, $lon2b, $lat2b) = @ARGV;


my $x1 = int ($w * ($lon1b - $lon1a) / ($lon2a - $lon1a));
my $x2 = int ($w * ($lon2b - $lon1a) / ($lon2a - $lon1a));
my $y1 = $h - int ($h * ($lat2b - $lat1a) / ($lat2a - $lat1a));
my $y2 = $h - int ($h * ($lat1b - $lat1a) / ($lat2a - $lat1a));


while (($x2 - $x1) % 8)
  {
    $x2++;
  }

while (($y2 - $y1) % 8)
  {
    $y2++;
  }

$lon2b = $lon1a + $x2 * ($lon2a - $lon1a) / $w;
$lat2b = $lat1a + ($h - $y1) * ($lat2a - $lat1a) / $h;

print &Dumper ([$lon2b, $lat2b]);

$lon2b = sprintf ('%.4f', $lon2b);
$lat2b = sprintf ('%.4f', $lat2b);


$p->Crop (geometry => sprintf ('%dx%d+%d+%d', $x2 - $x1, $y2 - $y1, $x1, $y1));

my $g = sprintf ('Landscape_%010.4f_%010.4f_%010.4f_%010.4f', $lon1b, $lat1b, $lon2b, $lat2b);

$p->Write ("$g.png");

'FileHandle'->new (">$g.opt")->print (<< "EOF");
--landscape.lonlat.position-{ --lat1 $lat1b --lon1 $lon1b --lat2 $lat2b --lon2 $lon2b }- --landscape.path landscape/$g.png 
EOF





