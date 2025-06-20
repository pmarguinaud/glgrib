package Tk::glGrib_Frame;

use Tk;
use Tk::Font;

use Tk::glGribBase qw (Tk::Frame);
use strict;

sub populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};
  $args->{'-relief'} = 'groove';

  
  my $opts = $self->{glGrib}{opts};
  my $name = $self->{glGrib}{name};

  my ($on);

  my @opts = @$opts;

  my $ftop = $self;

  my $bloc = ($opts[0] =~ m/^--/o) && ($opts[1] eq 'BLOCK');
  if ($bloc)
    {
      $ftop = $self->Frame (-relief => 'groove', -borderwidth => 2)
        ->pack (-side => 'top', -expand => 1, -fill => 'x');
      (undef, undef, my $desc, undef) = splice (@opts, 0, 4);
      &Tk::glGrib::h2 ($ftop, $desc);
    }


  if (@opts && ($opts[0] eq 'on'))
    {
      $on = &Tk::glGrib::create ($ftop, 'on', $opts[1]);
      splice (@opts, 0, 2);

      if (@opts)
        {
          $on->setCommand (sub { $on->getValue ()
                          ? $self->Enable () 
                          : $self->Disable (); }); 
        }
      $on->pack (-side => 'top', -fill => 'both', -side => 'top');
    }

  
  if (@opts)
    {
      my $frame = $self->{glGrib}{frame} = $ftop->Frame ();
     
      while (my ($key, $opt) = splice (@opts, 0, 2))
        {
          next if ($key eq 'on');
          my $w = &Tk::glGrib::create ($frame, $key, $opt);
          $w->pack (-side => 'top', -fill => 'both', -side => 'top');
          $frame->Separator (-width => 2)->pack (-side => 'top', -fill => 'x')
            if (@opts);
        }
     
      $self->Enable () if ((! $on) || $on->getValue ());

    }

  if ($bloc)
    {
      $ftop->Label (-text => ' ')->pack (-side => 'top', -fill => 'x');
    }

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

1;


