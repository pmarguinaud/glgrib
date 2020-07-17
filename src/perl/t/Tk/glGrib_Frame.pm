package Tk::glGrib_Frame;

use Tk;
use Tk::Font;

use tkbase qw (Tk::Frame);
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

  my $bloc = ($opts[0] =~ m/^--/o) && ($opts[1] eq 'BLOCK');
  if ($bloc)
    {

      (undef, undef, my $desc, undef) = splice (@opts, 0, 4);

      my $lab = 
      $self->Label (-text => $desc, -relief => 'groove')
        ->pack (-side => 'top', -expand => 1, -fill => 'x', -ipady => 5, -pady => 5);

      my $font = $lab->cget ('-font');
      $font =~ s{(\d+)}{int (1.5 * $1)}eo;
      $lab->configure (-font => $font);

    }


  if (@opts && ($opts[0] eq 'on'))
    {
      $on = &Tk::glGrib::create ($self, 'on', $opts[1]);
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
      my $frame = $self->{glGrib}{frame} = $self->Frame ();
     
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


