#!/usr/bin/perl -w

use strict;
use File::Find;
use File::Path;
use File::Basename;

&find ({wanted => sub 
{ 
  my $f = $File::Find::name; 
  my $b = &basename ($f);
  my $d = &dirname ($f);

  &rmtree ($f) if (($b eq '.packlist') || ($b eq 'perllocal.pod') || ($b eq '__pycache__')); 

}, no_chdir => 1}, 'debian/tmp');


