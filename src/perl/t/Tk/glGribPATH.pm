package Tk::glGribPATH;

use Tk;
use Tk::FileSelect;

use tkbase qw (Tk::Frame);
use tkbaselist;
use strict;

sub populate 
{
  my ($self, $args) = @_;
  
  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  $self->{glGrib} = delete $args->{glGrib};
  
  if (my $opts = $self->{glGrib}{opts})
    {
      $frame->Label (-text => $opts->[2])->pack (-side => 'top'); 
      $self->{variable} = \$opts->[3];
    }
  else
    {
      $self->{variable} = delete $args->{variable};
    }

  $frame->Button (-text => 'Browse', 
                  -command => sub { $self->selectPath (); })
    ->pack (-side => 'left');

  $frame->Entry (-textvariable => $self->{variable}, -width => 60)
    ->pack (-side => 'right', -expand => 1, -fill => 'x');

}

sub selectPath
{
  my ($self) = @_;

  my $select = $self->FileSelect (-directory => '.');

  my $path = $select->Show ();

  ${$self->getVariable ()} = $path;
}

sub getVariable
{
  my $self = shift;
  return $self->{variable};
}

sub set
{
  my ($self, $value) = @_;
  ${$self->getVariable ()} = $value;
}

1;
