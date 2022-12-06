package Tk::glGribDATE;

use Tk::glGribBase qw (Tk::glGrib_Entry);

sub eq
{
  shift;
  return $_[0] eq $_[1];
}

sub validate
{
  return 1;
}

1;
