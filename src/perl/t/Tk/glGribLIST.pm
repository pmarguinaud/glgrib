package Tk::glGribLIST;

use Tk;

use tkbase qw (Tk::Frame);
use strict;

sub eq
{
  my $class = shift;
  my ($v1, $v2) = @_;
  return unless (scalar (@$v1) == scalar (@$v2));
  $class = $class->class ();
  for my $i (0 .. $#{$v1})
    {
      return unless ($class->eq ($v1->[$i], $v2->[$i]));
    }
  return 1;
}

sub populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};
  
  my $o = $self->get_options ();

  $self->{width} = $o->{width} || 1;

  my $opts = $self->{glGrib}{opts};

  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'x');


  $frame->Button (-text => '+', -width => 4, -command => sub { $self->append () })
    ->pack (-side => 'left');
  $frame->Label (-text => $opts->[2])->pack (-side => 'left', -expand => 1, -fill => 'x');
  $frame->Button (-text => '-', -width => 4, -command => sub { $self->remove () })
    ->pack (-side => 'left');

  $self->{frame} = $self->Frame (-relief => 'groove', -borderwidth => 2)
    ->pack (-side => 'top', -expand => 1, -fill => 'both', -anchor => 'nw');


  $self->{number} = 0;
  $self->set ($self->{glGrib}{opts}[3]);
}

sub append
{
  my $self = shift;

  my $o = $self->{glGrib}{opts}[3];

  my $rank = $self->{number};

  if ($self->{number} == scalar (@$o))
    {
      push @$o, '';
    }
  
  my $class = $self->class ();

  my $len = $self->{width};
  my $num = $self->{number};
  my $row = int ($num / $len);
  my $col = $num - $row * $len;

  $self->{frame}->$class (variable => \$o->[$rank])
    ->grid (-row => $row, -column => $col, -sticky => 'nw');

  $self->{number}++;
     
}

sub remove
{
  my $self = shift;
  
  return unless ($self->{number});

  my $len = $self->{width};
  my $num = $self->{number}-1;
  my $row = int ($num / $len);
  my $col = $num - $row * $len;

  my ($c) = $self->{frame}->gridSlaves (-row => $row, -column => $col);

  my $o = $self->{glGrib}{opts}[3];
  pop (@$o);

  $c->gridForget ();

  $c->destroy ();

  $self->{number}--;
}

sub set
{
  my ($self, $value) = @_;

  while (1)
    {
      last if ($self->{number} == scalar (@$value));
      $self->append () if ($self->{number} < scalar (@$value));
      $self->remove () if ($self->{number} > scalar (@$value));
    } 
  
  my @c = $self->{frame}->children ();
  for (my $i = 0; $i < @$value; $i++)
    {
      my $e = $c[$i];
      my $text = $e->cget ('-textvariable');
      $$text = $value->[$i];
    }
}

1;


