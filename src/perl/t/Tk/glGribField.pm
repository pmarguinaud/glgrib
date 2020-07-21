package Tk::glGribField;

use tkbase qw (Tk::MainWindow);
use base qw (Tk::glGrib_Entity);
use strict;

use Tk::NoteBook;

our @TYPE = qw (scalar vector contour stream isofill);

sub populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};

  my $opts = $self->{glGrib}{opts};
  $self->saveOpts ();

  &Tk::glGrib::h1 ($self, ucfirst ($self->{glGrib}{name}));
  
  my $n = $self->{glGrib}{notebook} = 
  $self->NoteBook ()->pack (-expand => 1, -fill => 'both', -side => 'top');

  my $g = $self->{glGrib}{general} = $n->add ('general', -label => 'General');
  $g = $g->Scrolled ('Frame', -width => 500, -height => 600, -scrollbars => 'e', -sticky => 'nswe')->pack (-side => 'top', -expand => 1, -fill => 'both');

  my @opts = @$opts;


  my $selector;

  while (my ($key, $opt) = splice (@opts, 0, 2))
    {
      next if (grep { $_ eq $key } @TYPE);
      my $w = &Tk::glGrib::create ($g, $key, $opt);
      $w->pack (-side => 'top', -fill => 'both')
        unless ($w->isa ('Tk::MainWindow'));
      $w->Separator (-width => 2)->pack (-side => 'top', -fill => 'x')
        if (@opts);
      if ($key eq 'type')
        {
          $selector = $w;
        }
    }

  for my $type (@TYPE)
    {
      $self->{glGrib}{$type} =
      my $p = $n->add ($type, -label => ucfirst ($type));
 
      my $opt;
  
      my @opts = @$opts;

      while ((my $key, $opt) = splice (@opts, 0, 2))
        {
          last if ($key eq $type);
        }

      my $w = &Tk::glGrib::create ($p, $type, $opt);
      $w->pack (-side => 'top', -fill => 'both');

    }
  
  $self->createButtons ();

  $selector->setCommand (sub { $self->enableTab (@_) });

  $self->enableTab (lc ($selector->getValue ()));
}

sub enableTab
{
  my ($self, $type) = @_;

  $type = lc ($type);

  my @type = grep { $_ ne $type } @TYPE;

  $self->{glGrib}{notebook}->pageconfigure ($type, -state => 'normal');

  for my $type (@type)
    {
      $self->{glGrib}{notebook}->pageconfigure ($type, -state => 'disabled');
    }
  
}

1;

