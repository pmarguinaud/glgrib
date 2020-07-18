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

  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  $frame->Label (-text => $opts->[2])->pack (-side => 'left');

  $self->{glGrib}{button} = 
  $frame->Checkbutton (-variable => \$opts->[3])
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
  ${ $self->{glGrib}{button}->cget ('-variable') }
}

1;



