package glGrib;

use 5.024000;
use strict;
use warnings;

our $VERSION = '0.01';

use XSLoader;

&XSLoader::load ('glGrib', $VERSION);

my $done = 0;

sub import
{
  return if ($done);

  my %method = qw (start 0 snapshot 0 get 1 set 0 stop 0 json 1 window 1 list 1 resolve 1);

  while (my ($method, $return) = each (%method))
    {
      no strict 'refs';
      *{$method} = sub { use strict; shift; &glGrib::_method_ ($return, $method, @_); };
    }

  $done = 1;
}

1;

__END__

# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

glGrib - Perl extension for glGrib visualization

=head1 SYNOPSIS

  use glGrib;
  'glGrib'->start ('--grid.on', '--landscape.on');
  
  sleep (2);
  
  'glGrib'->set ('--grid.off');
  
  sleep (2);
  
  use Data::Dumper;
  
  print STDERR &Dumper (['glGrib'->get ('--view')]);
  
  'glGrib'->stop ();

=head1 DESCRIPTION

=head1 SEE ALSO

=head1 AUTHOR

Philippe Marguinaud, pmarguinaud@hotmail.com

=cut
