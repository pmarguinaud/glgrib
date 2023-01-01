use strict;
use warnings;
use FindBin qw ($Bin);
$ENV{GLGRIB_PREFIX} = "$Bin/../..";

use Test::More tests => 1;
BEGIN { use_ok('glGrib::glfw') };

'glGrib::glfw'->start ('--grid.on', '--landscape.on');

sleep (2);

'glGrib::glfw'->set ('--grid.off');

sleep (2);

use Data::Dumper;

print STDERR &Dumper (['glGrib::glfw'->get ('--view')]);

'glGrib::glfw'->stop ();

