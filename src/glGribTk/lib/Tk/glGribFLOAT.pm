package Tk::glGribFLOAT;

use Tk::glGribBase qw (Tk::glGrib_Entry);
use Tk::glGribBaselist (width => 4);

sub eq
{
  shift;
  return $_[0] == $_[1];
}

1;
