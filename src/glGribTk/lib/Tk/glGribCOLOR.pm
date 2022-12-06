package Tk::glGribCOLOR;

use tkbase qw (Tk::Frame);
use tkbaselist (width => 4);
use strict;

use Tk::glGribColorPicker;
use Tk::glGribColorPickerDB;

sub eq
{
  shift;
  return $_[0] eq $_[1];
}

sub populate 
{
  my ($self, $args) = @_;
  
  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  $self->{glGrib} = delete $args->{glGrib};

  if (my $opts = $self->{glGrib}{opts})
    {
      $frame->Label (-text => $opts->[2])->pack (-side => 'left', -expand => 1, -fill => 'x');
      $self->{variable} = \$opts->[3];
    }
  else
    {
      $self->{variable} = delete $args->{variable};
    }

  unless (${ $self->{variable} })
    {
      ${ $self->{variable} } = '#000000';
    }

  $self->{button} =
  $frame->Button (-width => 9, -height => 1)
    ->pack (-side => 'right', -fill => 'x', -expand => 1);
  $self->{button}->bind ('<Control-Button-1>' => sub { $self->chooseRGB (1); });
  $self->{button}->bind ('<Button-1>' => sub { $self->chooseRGB (0); });
  $self->setColor ();
}

sub setColor
{
  my $self = shift;

  my $color = ${ $self->{variable} };

  if ($color =~ m/^(#[a-f0-9]{6})/o)
    {
      my $c = $1;
      $self->{button}->configure (-background => $c);
    }

}

sub getRGB
{
  my ($self, $db) = @_;
  my $picker = $db
             ? 'Tk::glGribColorPickerDB'->new (-db => 'glGrib'->resolve ('glGrib.db'))
             : 'Tk::glGribColorPicker'->new (-color => ${ $self->{variable} });
  my $color = $picker->Show ();

  $picker->destroy ();
  return $color;
}

sub chooseRGB
{
  my ($self, $db) = @_;
  ${ $self->{variable} } = $self->getRGB ($db);
  $self->setColor ();
}

sub set
{
  my ($self, $value) = @_;
  ${ $self->{variable} } = $value;
  $self->setColor ();
}


1;
