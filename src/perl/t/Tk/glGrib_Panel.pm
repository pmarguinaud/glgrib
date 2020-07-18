package Tk::glGrib_Panel;

use Tk;

use tkbase qw (Tk::MainWindow);
use strict;

sub populate 
{
  my ($self, $args) = @_;

  $self->{glGrib} = delete $args->{glGrib};
  
  my $opts = $self->{glGrib}{opts};
  $self->{glGrib}{opts_} = &Storable::dclone ($opts);

  my $frame = $self->Scrolled ('Frame', -width => 400, -height => 400, -scrollbars => 'e', -sticky => 'nswe')->pack (-expand => 1, -fill => 'both');

  &Tk::glGrib::h1 ($frame, ucfirst ($self->{glGrib}{name}));

  $frame = $frame->Frame ()->pack (-expand => 1, -fill => 'both', -side => 'top');

  my (@sep, $on);
  my @opts = @$opts;

  if (@opts && ($opts[0] eq 'on'))
    {
      $on = &Tk::glGrib::create ($frame, 'on', $opts[1]);
      splice (@opts, 0, 2);
   
      $on->setCommand (sub { $on->getValue ()
                      ? $self->Enable () 
                      : $self->Disable (); });
      $on->pack (-side => 'top', -fill => 'both', -side => 'top');
      $on->Separator (-width => 2)->pack (-side => 'top', -fill => 'x')
        if (@opts);
    }

  $frame = $self->{glGrib}{frame} = $frame->Frame ();


  while (my ($key, $opt) = splice (@opts, 0, 2))
    {
      my $w = &Tk::glGrib::create ($frame, $key, $opt);
      $w->pack (-side => 'top', -fill => 'both', -side => 'top');
      $w->Separator (-width => 2)->pack (-side => 'top', -fill => 'x')
        if (@opts);
    }

  $self->Enable () if ((! $on) || $on->getValue ());

  my $b = 
  $self->Button (-relief => 'raised', -text => 'Apply', -width => 12,
                 -command => sub { $self->Apply (); })
  ->pack (-side => 'left', -expand => 1, -fill => 'x');
  $self->Button (-relief => 'raised', -text => 'Close', -width => 12,
                 -command => sub { $self->destroy (); })
  ->pack (-side => 'left', -expand => 1, -fill => 'x');
  $self->Button (-relief => 'raised', -text => 'Apply/Close', -width => 12,
                 -command => sub { $self->Apply (); $self->destroy (); })
  ->pack (-side => 'left', -expand => 1, -fill => 'x');

}

sub Enable
{
  my $self = shift;
  $self->{glGrib}{frame}->pack (-expand => 1, -fill => 'both', -side => 'top');
}

sub Disable
{
  my $self = shift;
  $self->{glGrib}{frame}->packForget ();
}

sub Apply
{
  my $self = shift;

  my @opts = &Tk::glGrib::diffOptions ($self->{glGrib}{opts_}, $self->{glGrib}{opts});

  print &Data::Dumper::Dumper (\@opts);
  'glGrib'->set (@opts);

  $self->{glGrib}{opts_} = &Storable::dclone ($self->{glGrib}{opts});

}

1;


