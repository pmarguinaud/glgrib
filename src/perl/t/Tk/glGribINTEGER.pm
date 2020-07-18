package Tk::glGribINTEGER;

use tkbase qw (Tk::glGrib_Entry);
use tkbaselist;

sub validate
{
  my $self = shift;
  my ($value) = @_;
  return $value =~ m/^\d+$/o;
}

sub eq
{
  shift;
  return $_[0] == $_[1];
}

1;
