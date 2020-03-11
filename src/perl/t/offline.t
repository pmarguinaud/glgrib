use strict;
use warnings;

use Test::More tests => 1;
BEGIN { use_ok('glGrib') };

unlink ($_) for (<snapshot*.png>);

'glGrib'->start ('--grid.on', '--landscape.on', '--window.offscreen.on');
'glGrib'->snapshot ();
'glGrib'->set ('--grid.off');
'glGrib'->snapshot ();
'glGrib'->stop ();


