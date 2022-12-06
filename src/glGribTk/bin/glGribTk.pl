#!/usr/bin/perl -w

use strict;
use warnings;

use Tk;
use Tk::glGribMainWindow;

$ENV{GLGRIB_PREFIX} ||= "/usr";

my $main = 'Tk::glGribMainWindow'->new (-opts => \@ARGV);

$main->geometry ('+0+0');

&MainLoop ();


