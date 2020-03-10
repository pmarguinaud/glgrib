# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl glGrib.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use strict;
use warnings;

use Test::More tests => 1;
BEGIN { use_ok('glGrib') };

#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.


&glGrib::start ('--grid.on', '--landscape.on');

sleep (2);

&glGrib::set ('--grid.off');

sleep (2);

use Data::Dumper;

print STDERR &Dumper ([&glGrib::get ('--view')]);

&glGrib::stop ();

