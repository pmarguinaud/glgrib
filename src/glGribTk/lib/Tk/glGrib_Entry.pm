package Tk::glGrib_Entry;

use Tk;

use Tk::glGribBase qw (Tk::Frame);
use strict;

sub eq
{
  shift;
  return $_[0] eq $_[1];
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
  $frame->Entry (-textvariable => $self->getVariable (), -validate => 'key',
                 -validatecommand => sub { $self->validate (@_) }, -width => 12)
    ->pack (-side => 'right');

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


