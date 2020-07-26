
use strict;
use warnings;

use FindBin qw ($Bin);
use lib $Bin;
use File::Basename;

for my $x (<$Bin/Tk/*.pm>)
  {
    $x = &basename ($x, qw (.pm));
    eval "use Tk::$x";
    if (my $c = $@)
      {
        die $c;
      }
  }


use Tk;

$ENV{GLGRIB_PREFIX} = "$Bin/../..";

my $main = 'Tk::glGribMainWindow'->new ();

$main->geometry ('+0+0');

&MainLoop ();


