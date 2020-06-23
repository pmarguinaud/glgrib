package tkbaselist;

use strict;

sub import
{
  shift;

  my $class = (caller (0))[0];

  my $code = << "EOF";
package ${class}LIST;

use strict;

use tkbase qw ($class);

sub validate
{
  my \$self = shift;
  my \@value = split (m/\s+/o, \$_[0]);

  for (\@value)
    {
      return 0 unless (\$self->SUPER\::validate (\$_));
    }
  
  \@{ \$self->{glGrib}{opts}[3] } = \@value;

  return 1;
}

sub populate
{
  my \$self = shift;

  \$self->{value} = "";
  \$self->$class\::populate (\@_);

  \$self->{value} = join (' ', \@{ \$self->{glGrib}{opts}[3] });
  return \$self;
}

sub getVariable
{
  my \$self = shift;
  return \\\$self->{value};
}

1;

EOF

  eval $code;

  my $c = $@;

  $c && die ($c);

}

1;
