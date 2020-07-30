
use strict;
use warnings;


use FindBin qw ($Bin);

use Tk;
use Tk::glGribMainWindow;

$ENV{GLGRIB_PREFIX} = "$Bin/../..";

my $main = 'Tk::glGribMainWindow'->new (-opts => \@ARGV);

$main->geometry ('+0+0');

&MainLoop ();


