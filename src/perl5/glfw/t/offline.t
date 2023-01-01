use strict;
use warnings;
use FindBin qw ($Bin);
$ENV{GLGRIB_PREFIX} = "$Bin/../..";

use Test::More tests => 1;
BEGIN { use_ok('glGrib::glfw') };

unlink ($_) for (<snapshot*.png>);

'glGrib::glfw'->start ('--grid.on', '--landscape.on', '--render.offscreen.on');
'glGrib::glfw'->snapshot ();
'glGrib::glfw'->set ('--grid.off');
'glGrib::glfw'->snapshot ();
'glGrib::glfw'->stop ();


