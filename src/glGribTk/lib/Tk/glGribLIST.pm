package Tk::glGribLIST;

use Tk;

use Tk::glGribBase qw (Tk::Frame);
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


  $self->{children} = [];
  $self->set ($self->{glGrib}{opts}[3]);
}

sub regrid
{
  my $self = shift;

  my @c = @{ $self->{children} };

  my ($row, $col) = (0, 0);
  for my $i (0 .. $#c)
    {
      $c[$i]->grid (-row => $row, -column => $col, -sticky => 'nswe');
      my ($b) = $c[$i]->children ();
      $b->bind ('<Button-1>'         => sub { $self->splice ($i); });
      $b->bind ('<Control-Button-1>' => sub { $self->insert ($i); });
      $col++;
      if ($col % $self->{width} == 0)
        {
          $col = 0;
          $row++;
        }
    }

}

sub splice : method
{
  my ($self, $rank) = @_;

  my ($c) = $self->{children}[$rank];

  my $o = $self->{glGrib}{opts}[3];
  splice (@$o, $rank, 1);
  splice (@{ $self->{children} }, $rank, 1);

  $c->gridForget ();

  $c->destroy ();

  $self->regrid ();
}

sub insert
{
  my ($self, $rank) = @_;

  my $o = $self->{glGrib}{opts}[3];

  splice (@$o, $rank, 0, '');

  my $class = $self->class ();

  my $f = $self->{frame}->Frame ();

  my $b = 
  $f->Button (-text => '.')
    ->pack (-side => 'left', -expand => 1);

  $f->$class (variable => \$o->[$rank])
    ->pack (-side => 'right', -fill => 'x', -expand => 1);

  splice (@{ $self->{children} }, $rank, 0, $f);

  $self->regrid ();
}

sub append
{
  my $self = shift;

  my $o = $self->{glGrib}{opts}[3];

  my $rank = scalar (@{ $self->{children} });

  if ($rank == scalar (@$o))
    {
      push @$o, '';
    }
  
  my $class = $self->class ();

  my $f = $self->{frame}->Frame ();

  my $b = 
  $f->Button (-text => '.')
    ->pack (-side => 'left', -expand => 1);

  $f->$class (variable => \$o->[$rank])
    ->pack (-side => 'right', -fill => 'x', -expand => 1);

  push @{ $self->{children} }, $f;

  $self->regrid ();

}

sub remove
{
  my $self = shift;
  
  return unless (@{ $self->{children} });

  my $c = pop (@{ $self->{children} });
  pop (@{ $self->{glGrib}{opts}[3] });

  $c->gridForget ();

  $c->destroy ();
}

sub set
{
  my ($self, $value) = @_;

  while (1)
    {
      last if (scalar (@{ $self->{children} }) == scalar (@$value));
      $self->append () if (scalar (@{ $self->{children} }) < scalar (@$value));
      $self->remove () if (scalar (@{ $self->{children} }) > scalar (@$value));
    } 
  
  my @c = @{ $self->{children} };
  for (my $i = 0; $i < @$value; $i++)
    {
      my $e = $c[$i];
      (undef, $e) = $e->children ();
      my $text = $e->cget ('-textvariable');
      $$text = $value->[$i];
    }
}

1;


