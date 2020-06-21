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

1;

EOF

  eval $code;

  my $c = $@;

  $c && die ($c);

}

1;
