package Tk::glGrib_Popup;

use Tk;

use tkbase qw (Tk::Frame);
use strict;

sub list
{
  my $self = shift;
  my $class = ref ($self);
  no strict 'refs';
  return @{"$class\::LIST"};
}

sub populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};
 
  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  if (my $opts = $self->{glGrib}{opts})
    {
      $frame->Label (-text => $opts->[2])->pack (-side => 'top');
      $self->{variable} = \$opts->[3];
    }
  else
    {
      $self->{variable} = delete $args->{variable};
    }
  
  my $button;

  $button =
  $frame->Button (-textvariable => $self->{variable}, -command => sub
  {
    my ($x, $y) = $button->pointerxy ();
    my $menu = $button->Menu
    (
      -tearoff => 0, 
      -menuitems =>  
      [
        map 
        { 
          my $p = $_;
          [command => $p, -command => sub { ${$self->{variable}} = $p; }]  
        } $self->list ()
      ],
    );
    $menu->post ($x, $y);
  })->pack (-side => 'top', -expand => 1, -fill => 'x');
  
 

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


