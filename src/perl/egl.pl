#!/usr/bin/perl -w

use strict;
use FileHandle;
use File::Path;
use File::Copy;
use Data::Dumper;
use File::Basename;

my ($src, $dst) = ('glfw', 'egl');

chomp for (my @manifest = do { my $fh = 'FileHandle'->new ("<$src/MANIFEST"); <$fh> });

&rmtree ($dst);
&mkpath ($dst);

for my $f (@manifest)
  {
    &mkpath (&dirname ("$dst/$f"));
    my $text = do { my $fh = 'FileHandle'->new ("<$src/$f"); local $/ = undef; <$fh> };
    for ($f, $text)
      {
        s/\bglfw\b/egl/goms;
      }
    'FileHandle'->new (">$dst/$f")->print ($text);
  }



