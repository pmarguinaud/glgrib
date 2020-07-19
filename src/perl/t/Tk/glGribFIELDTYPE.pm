package Tk::glGribFIELDTYPE;


use Tk;

use strict;
use tkbase qw (Tk::Frame);

sub eq
{
  shift;
  return $_[0] eq $_[1];
}

sub populate 
{
  my ($self, $args) = @_;

  $self->{glGrib} = delete $args->{glGrib};

  my @type = @Tk::glGribField::TYPE;

  if (my $opts = $self->{glGrib}{opts})
    {
      $self->{variable} = \$opts->[3];
    }
  else
    {
      $self->{variable} = delete $args->{variable};
    }
  

  for my $type (@type)
    {
      $self->Radiobutton 
      (
        -text => $type,
        -value => uc ($type),
        -variable => $self->{variable},
        -command => sub { $self->onCommand () },
      )->pack (-side => 'left', -fill => 'x');
    }
}

sub getVariable
{
  my $self = shift;
  return $self->{variable};
}

sub getValue
{
  my $self = shift;
  return ${ $self->{variable} };
}

sub onCommand
{
  my $self = shift;
  $self->{command}->(${ $self->{variable} }) if ($self->{command});
}

sub setCommand
{
  my $self = shift;
  $self->{command} = $_[0];
}

sub set
{
  my ($self, $value) = @_;
  ${ $self->{variable} } = $value;
  $self->onCommand ();
}


1;
