package Tk::glGribLIST;

use Tk;

use tkbase qw (Tk::Frame);
use strict;

sub populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};
  
  my $opts = $self->{glGrib}{opts};
  my $name = $self->{glGrib}{name};

  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  $frame->Label (-text => $opts->[2])->pack (-side => 'top');

  $self->{frame} = $frame->Frame ()
    ->pack (-side => 'top', -expand => 1, -fill => 'both');


  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');
  $frame->Button (-text => '+', -command => sub { $self->append () })
    ->pack (-side => 'left', -expand => 1, -fill => 'x');
  $frame->Button (-text => '-', -command => sub { $self->remove () })
    ->pack (-side => 'right', -expand => 1, -fill => 'x');
  $self->set ($self->{glGrib}{opts}[3]);

}

sub append
{
  my $self = shift;

  my $o = $self->{glGrib}{opts}[3];

  my @c = $self->{frame}->children ();
  my $rank = scalar (@c);

  if (scalar (@c) == scalar (@$o))
    {
      push @$o, '';
    }
  
  my $class = $self->class ();

  my $frame = $self->{frame}->$class (variable => \$o->[$rank])
    ->pack (-side => 'top', -expand => 1, -fill => 'both');

     
}

sub remove
{
  my $self = shift;
  
  my @c = $self->{frame}->children ();

  return unless (@c);

  my $c = pop (@c);

  my $o = $self->{glGrib}{opts}[3];
  pop (@$o);


  $c->packForget ();

  $c->destroy ();

}

sub set
{
  my ($self, $value) = @_;

  while (1)
    {
      my @c = $self->{frame}->children ();
      last if (scalar (@c) == scalar (@$value));
      $self->append () if (scalar (@c) < scalar (@$value));
      $self->remove () if (scalar (@c) > scalar (@$value));
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

package tkbaselist;

use strict;

sub import
{
  shift;

  my $class = (caller (0))[0];

  my $code = << "EOF";
package ${class}LIST;

use tkbase qw (Tk::glGribLIST);

sub class
{
  shift;
  return \"$class\";
}

EOF

  eval $code;

  my $c = $@;

  $c && die ($c);

}

1;
