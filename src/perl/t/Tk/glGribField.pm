package Tk::glGribField;

use tkbase qw (Tk::MainWindow);
use strict;

sub populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};

  my @type = qw (scalar vector contour stream isofill);

  my $opts = $self->{glGrib}{opts};
  $self->{glGrib}{opts_} = &Storable::dclone ($self->{glGrib}{opts});

  &Tk::glGrib::h1 ($self, ucfirst ($self->{glGrib}{name}));
  
  my $n = $self->{glGrib}{notebook} = 
  $self->NoteBook ()->pack (-expand => 1, -fill => 'both', -side => 'top');

  my $g = $self->{glGrib}{general} = $n->add ('general', -label => 'General');
  $g = $g->Scrolled ('Frame', -width => 500, -height => 600, -scrollbars => 'e', -sticky => 'nswe')->pack (-side => 'top', -expand => 1, -fill => 'both');

  my @opts = @$opts;

  my $tt;

  for (my $i = 0; $i < $#{$opts}; $i++)
    {
      if ($opts->[$i] eq 'type')
        {
          $tt = \$opts->[$i+1][3];
          last;
        }
    }

  my $fb = $g->Frame ()->pack (-expand => 1, -fill => 'x', -side => 'top');

  for my $type (@type)
    {
      $fb->Radiobutton 
      (
        -text => $type,
        -value => uc ($type),
        -variable => $tt,
        -command => sub { $self->enableTab ($type) },
      )->pack (-side => 'left', -fill => 'x');
    }

  while (my ($key, $opt) = splice (@opts, 0, 2))
    {
      next if (grep { $_ eq $key } @type);
      next if ($key eq 'type');
      my $w = &Tk::glGrib::create ($g, $key, $opt);
      $w->pack (-side => 'top', -fill => 'both')
        unless ($w->isa ('Tk::MainWindow'));
      $w->Separator (-width => 2)->pack (-side => 'top', -fill => 'x')
        if (@opts);
    }

  for my $type (@type)
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
      $w->pack (-side => 'top', -fill => 'both')
        unless ($w->isa ('Tk::MainWindow'));

    }
  
  $self->Button (-relief => 'raised', -text => 'Apply', -width => 12,
                 -command => sub { $self->Apply () })
  ->pack (-side => 'left', -fill => 'x', -expand => 1);
  $self->Button (-relief => 'raised', -text => 'Close', -width => 12,
                 -command => sub { $self->destroy (); })
  ->pack (-side => 'left', -fill => 'x', -expand => 1);
  $self->Button (-relief => 'raised', -text => 'Apply/Close', -width => 12,
                 -command => sub { $self->Apply (); $self->destroy (); })
  ->pack (-side => 'left', -expand => 1, -fill => 'x');

  $self->enableTab (lc ($$tt));
}

sub enableTab
{
  my ($self, $type) = @_;

  my @type = qw (scalar vector contour stream isofill);
  @type = grep { $_ ne $type } @type;

  $self->{glGrib}{notebook}->pageconfigure ($type, -state => 'normal');

  for my $type (@type)
    {
      $self->{glGrib}{notebook}->pageconfigure ($type, -state => 'disabled');
    }
  
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


