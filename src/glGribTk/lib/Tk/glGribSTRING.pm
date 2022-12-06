package Tk::glGribSTRING;

use Tk::glGribBase qw (Tk::glGrib_Entry);
use Tk::glGribBaselist;

sub eq
{
  shift;
  return $_[0] eq $_[1];
}

1;
