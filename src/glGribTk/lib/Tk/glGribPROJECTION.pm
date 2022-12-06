package Tk::glGribPROJECTION;

use Tk::glGribBase qw (Tk::glGrib_Popup);
our @LIST = qw (XYZ POLAR_NORTH POLAR_SOUTH MERCATOR LONLAT);

sub eq
{
  shift;
  return $_[0] eq $_[1];
}

1;
