package Tk::glGribBaselist;

use strict;
use Tk::glGribLIST;
use Data::Dumper;

sub import
{
  shift;
  my %args = @_;

  my $args = do { local $Data::Dumper::Terse = 1; &Dumper (\%args) };

  my $class = (caller (0))[0];

  my $code = << "EOF";
package ${class}LIST;

use Tk::glGribBase qw (Tk::glGribLIST);

sub class
{
  shift;
  return \"$class\";
}

sub get_options
{
  return $args;
}

EOF

  eval $code;

  my $c = $@;

  $c && die ($c);

}

1;
