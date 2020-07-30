package Tk::glGribSTRING;

use tkbase qw (Tk::glGrib_Entry);
use tkbaselist;

sub eq
{
  shift;
  return $_[0] eq $_[1];
}

1;
