package Tk::glGribPALETTENAME;

use Tk;

use tkbase qw (Tk::Frame);
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
  
  $self->{button} =
  $frame->Button (-textvariable => $self->getVariable (), -width => 20,
                  -command => sub { $self->choosePalette () })
    ->pack (-side => 'right', -fill => 'x', -expand => 1);

  $self->setImage ();

  return $self;
}

sub setImage
{
  my $self = shift;

  my $image20 = &Tk::PalettePickerDB::getPaletteGif ($self, ${$self->{variable}});

  if ($image20)
    {
      $self->{button}->configure (-image => $image20, -text => undef);
    }
  else
    {
      $self->{button}->configure (-text => ${$self->{variable}}, -image => undef);
    }
}

sub choosePalette
{
  my $self = shift;
  use Tk::PalettePickerDB;

  my $db = 'glGrib'->resolve ('glGrib.db');

  my $pick = $self->PalettePickerDB 
     (-db => $db, -palette => ${$self->{variable}}, -title => 'Palette');

  ${$self->{variable}} = $pick->Show ();

  $self->setImage ();
}

sub getVariable
{
  my $self = shift;
  return $self->{variable};
}

sub set
{
  my ($self, $value) = @_;
  ${ $self->getVariable () } = $value;
}

1;


