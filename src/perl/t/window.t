use strict;
use warnings;

use Test::More tests => 1;
BEGIN { use_ok('glGrib') };

'glGrib'->start ('--grid.on', '--landscape.on');

sleep (2);

'glGrib'->set ('--grid.off');

sleep (2);

use Data::Dumper;

print STDERR &Dumper (['glGrib'->get ('--view')]);

'glGrib'->stop ();

