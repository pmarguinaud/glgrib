package glGrib::glfw;

use 5.016000;
use strict;
use warnings;

our $VERSION = '0.01';

use XSLoader;

&XSLoader::load ('glGrib::glfw', $VERSION);

our $DONE;

sub import
{
  my $class = shift;

  {
    no strict 'refs';
    return if (${"$class\::DONE"}++);
  }

  my %method = qw (start 0 snapshot 0 get 1 set 0 stop 0 json 1 window 1 list 1 resolve 1 clone 0);

  while (my ($method, $return) = each (%method))
    {
      no strict 'refs';
      *{"$class\::$method"} = sub { shift; &{"$class\::_method_"} ($return, $method, @_); };
    }

}

1;

__END__

# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

glGrib::glfw - Perl extension for glGrib visualization

=head1 SYNOPSIS

  use glGrib::glfw;
  'glGrib::glfw'->start ('--grid.on', '--landscape.on');
  
  sleep (2);
  
  'glGrib::glfw'->set ('--grid.off');
  
  sleep (2);
  
  use Data::Dumper;
  
  print STDERR &Dumper (['glGrib::glfw'->get ('--view')]);
  
  'glGrib::glfw'->stop ();

=head1 DESCRIPTION

=head1 SEE ALSO

=head1 AUTHOR

Philippe Marguinaud, pmarguinaud@hotmail.com

=cut
