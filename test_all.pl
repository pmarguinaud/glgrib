#!/usr/bin/perl -w
#
use strict;
use FileHandle;
use Data::Dumper;

my @make = grep { !/^test_all:/o } do { my $fh = 'FileHandle'->new ("<makefile"); <$fh> };
pop (@make) while ($make[-1] =~ m/^\s*$/o);

my @test = grep { $_ ne 'test_all' } map { m/^(test_\w+):/o ? ($1) : () } @make;

'FileHandle'->new (">makefile")->print (join ('', @make, "\ntest_all: @test\n"));



