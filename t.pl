#!/usr/bin/perl -w

use strict;
use FileHandle;
use Data::Dumper;

my $text = do { my $fh = 'FileHandle'->new ('<makefile'); local $/ = undef; <$fh> };

my %test = ($text =~ m/^(test_\w+):\n((?:\s+\S.*\n)+?)\n/gom);

my %t;

while (my ($key, $val) = each (%test))
  {
    $key =~ s/^test_//o;
    for ($val)
      {
        s/(?:^\s*|\s*$)//goms;
        s/^\$\(RUNTEST\)\s*//o;
        s/\\\n//gom;
        s/\s+/ /goms;
      }
    chomp ($val);
    $t{$key} = $val;
  }

print &Dumper (\%t);

