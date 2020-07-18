package Tk::glGribTRANSFORMATION;

use tkbase qw (Tk::glGrib_Popup);
our @LIST = qw (PERSPECTIVE ORTHOGRAPHIC);

sub eq
{
  shift;
  return $_[0] eq $_[1];
}

1;
