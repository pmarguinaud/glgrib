#!/usr/bin/perl -w
#

use warnings FATAL => 'all';
use strict;
use FileHandle;
use File::Basename;

sub load
{
  my @text = do { my $fh = 'FileHandle'->new ("<$f"); <$fh> };
  return @text;
}

for my $f (<shaders/src/*.vs>, <shaders/src/*.fs>)
  {
    my @text = &load ($f);
    my @code;
    for my $text (@text)
      {
        if ($text =~ m/^#include\s+"([^"]+)"\s*$/o)
          {
            my $h = $1;
            push @code, "// start include $h\n", &load ($h), "// end include $h\n";
	  }
	else
	  {
            push @code, $text;
	  }
      }
    'FileHandle'->new (">shaders/$f")->print (join ('', @code));
  }
