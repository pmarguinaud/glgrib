package Tk::glGribINTEGER;

use Tk::glGribBase qw (Tk::glGrib_Entry);
use Tk::glGribBaselist;

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
