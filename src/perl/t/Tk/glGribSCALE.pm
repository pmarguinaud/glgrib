package Tk::glGribSCALE;

use Tk;

use tkbase qw (Tk::Frame);
use strict;

sub eq
{
  shift;
  return $_[0] == $_[1];
}

sub populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};
 
  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  if (my $opts = $self->{glGrib}{opts})
    {
      $frame->Label (-text => $opts->[2])->pack (-side => 'left');
      $self->{variable} = \$opts->[3];
    }
  else
    {
      $self->{variable} = delete $args->{variable};
    }
  
  $self->{entry} =
  $frame->Scale (-variable => $self->getVariable (), -from => 0.95, -to => 1.05, 
                 -showvalue => 1, -orient => 'horizontal', -resolution => 0.01)
    ->pack (-side => 'right', -expand => 1, -fill => 'x');

  return $self;
}

sub getVariable
{
  my $self = shift;
  return $self->{variable};
}

sub validate
{
  my $self = shift;
  return 1;
}

sub set
{
  my ($self, $value) = @_;
  ${ $self->getVariable () } = $value;
}

1;



