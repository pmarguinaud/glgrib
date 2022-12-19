#!/usr/bin/perl -w

use strict;
use File::Find;
use File::Basename;

&find ({wanted => sub { my $f = $File::Find::name; unlink ($f) if ((&basename ($f) eq '.packlist') || (&basename ($f) eq 'perllocal.pod')); }, no_chdir => 1}, 'debian/tmp');


