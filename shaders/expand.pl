#!/usr/bin/perl -w
#

use warnings FATAL => 'all';
use strict;
use FileHandle;
use File::Basename;
use Data::Dumper;

sub load
{
  my $f = shift;
  my @text = do { my $fh = 'FileHandle'->new ("<$f"); <$fh> };
  return @text;
}

sub expand
{
  my @text = @_;
  my @code;
  for my $text (@text)
    {
      if ($text =~ m/^#include\s+"([^"]+)"\s*$/o)
        {
          my $h = $1;
          push @code, "// start include $h\n", &load ("shaders/include/$h"), "// end include $h\n";
        }
      else
        {
          push @code, $text;
        }
    }
  return @code;
}

mkdir ('.shaders');

for my $f (<shaders/src/*.vs>, <shaders/src/*.fs>, <shaders/src/*.gs>)
  {
    my @text = &load ($f);
    my @code;
    while (1)
      { 
        @code = &expand (@text);
        last if ("@code" eq "@text");
        @text = @code;
      }
    $f = &basename ($f);
    'FileHandle'->new (">.shaders/$f")->print (join ('', @code));
  }
