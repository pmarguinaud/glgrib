package Tk::glGribBOOLEAN;

use Tk;

use tkbase qw (Tk::Frame);
use strict;

sub eq
{
  shift;
  return ($_[0] && $_[1]) || ((! $_[0]) && (! $_[1]));
}

sub populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};
  
  my $opts = $self->{glGrib}{opts};
  my $name = $self->{glGrib}{name};

  $self->{variable} = \$opts->[3];

  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  $frame->Label (-text => $opts->[2])->pack (-side => 'left');

  $self->{glGrib}{button} = 
  $frame->Checkbutton (-variable => $self->{variable})
    ->pack (-side => 'right');

}

sub setCommand
{
  my ($self, $command) = @_;
  $self->{glGrib}{button}
    ->configure (-command => $command);
}

sub getValue
{
  my $self = shift;
  ${ $self->{variable} };
}

sub getVariable
{
  my $self = shift;
  $self->{variable};
}

sub set
{
  my ($self, $value) = @_;

  my $v = $self->getVariable ();

  if (! $self->eq ($self->getValue (), $value))
    {
      $self->{glGrib}{button}->invoke ();
    }
}

1;



