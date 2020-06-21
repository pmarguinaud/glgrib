package tkbase;

use strict;
use Data::Dumper;

sub import
{
  shift;
  my ($base) = @_;

  my $class = (caller (0))[0];

  (my $name = $class) =~ s/^Tk:://o;
  
  $base =~ s/^Tk:://o;

  my $code = << "EOF";
package Tk;

use strict;

sub $name
{
  my \$self = shift;

  my \%args = \@_;

  my \$base = '$class'->tkbase ();

  my \$object = \$self->\$base ();

  bless \$object, '$class';

  my \$args = \\\%args;

  \$object->populate (\$args);

  \$object->configure (%\$args);

  return \$object;
}

1;

package Tk::$name;

use strict;
our \@ISA = qw (Tk::$base);

sub tkbase
{
  my \$base = 'Tk::$base';
  while (\$base->can ('tkbase'))
    {
      \$base = \$base->tkbase ();
    }
  \$base =~ s/^Tk:://o;
  return \$base;
}

sub new
{
  shift;
  my \$args = {\@_};
  my \$self = 'Tk::MainWindow'->new ();
  bless \$self, '$class';
  \$self->populate (\$args);
  \$self->configure (%\$args);
  return \$self;
}

1;

EOF

  eval $code;

  my $c = $@;

  $c && die ($c);

}

1;
