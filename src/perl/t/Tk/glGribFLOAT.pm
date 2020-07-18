package Tk::glGribFLOAT;

use tkbase qw (Tk::glGrib_Entry);
use tkbaselist (width => 5);

sub eq
{
  shift;
  return $_[0] == $_[1];
}

1;
