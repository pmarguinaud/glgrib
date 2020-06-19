package TkglGrib;

use strict;
use JSON;

sub json2tree
{
  my $list = shift;

  my $v = [];

  for (@$list)  
    {
      my ($name, $type, $desc, $vals) = @$_;

      $name =~ s/^--//o;
      my @name = split (m/\./o, $name);
      
      if (ref ($vals) =~ m/Boolean/o)
        {
          $vals = $vals ? 1 : 0;
        } 

      my $c = \$v;
      for my $n (@name)
        {
          unless ((scalar (@{${$c}}) > 0) && (${$c}->[-2] eq $n))
            {
              push @{${$c}}, ($n, []);
            }
          $c = \${$c}->[-1];
        }

      $$c = ["--$name", $type, $desc, $vals];

    }

  return $v;
}

sub base
{
  my $json = do { my $fh = 'FileHandle'->new ('<glgrib.json'); local $/ = undef; <$fh> };
  return &json2tree (&decode_json ($json));
# return &json2tree (&decode_json ('glGrib'->json ('+base', $_[0])));
}

sub exists 
{
  my $class = shift;
  no strict 'refs';
  return scalar (@{"$class\::ISA"});
}

sub isMainWindow
{
  my $class = shift;
  return $class->isa ('Tk::MainWindow');
}

sub create
{
  my ($win, $name, $opts, $default, @args) = @_;


  my $class = 'glGrib' . ucfirst ($name);
  $class =~ s/[^a-z]+$//io;
  $class =~ s{_(\w)}{uc ($1)}egoms;

  if ((ref ($opts) eq 'ARRAY') && (scalar (@$opts) > 0)  
   && (! $default) && (ref ($opts->[1]) ne 'ARRAY'))
    {
      my $type = $opts->[1];
      $type =~ s/[^A-Z]//go;
      $default = "glGrib$type" if (&exists ("Tk::glGrib$type"));
    }

  $class = $default 
    unless (&exists ("Tk::$class"));

  $class ||= 'glGrib_Frame';

  if ($class)
    {
      return &isMainWindow ("Tk::$class")
           ? "Tk::$class"->new (glGrib => {name => $name, opts => $opts}, 
                                -title => ucfirst ($name), @args)
           : $win->$class (glGrib => {name => $name, opts => $opts}, @args);
    }

  

  my $w = $win->Frame (-label => $name);

  


  return $w;
}

package Tk::glGrib_Frame;

use Tk;

use base qw (Tk::Frame);
use strict;

Construct Tk::Widget 'glGrib_Frame';

sub ClassInit 
{
  my ($class, $mw) = @_;
  $class->SUPER::ClassInit ($mw);
}

sub Populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};
  
  my $opts = $self->{glGrib}{opts};
  my $name = $self->{glGrib}{name};

  $self->SUPER::Populate ($args);
  
  if (scalar (@$opts) == 1)
    {
      my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');
      $frame->Label (-text => $name)->pack (-side => 'top');
    }

  my @opts = @$opts;

  while (my ($key, $opt) = splice (@opts, 0, 2))
    {
      my $w = &TkglGrib::create ($self, $key, $opt);
      $w->pack (-side => 'top', -fill => 'both', -side => 'top')
        unless ($w->isa ('Tk::MainWindow'));
    }
}

1;

package Tk::glGrib_Entry;

use Tk;

use base qw (Tk::Frame);
use strict;

Construct Tk::Widget 'glGrib_Entry';

sub ClassInit 
{
  my ($class, $mw) = @_;
  $class->SUPER::ClassInit ($mw);
}

sub Populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};
  
  my $opts = $self->{glGrib}{opts};
  my $name = $self->{glGrib}{name};

  $self->SUPER::Populate ($args);
  
  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  $frame->Label (-text => $opts->[2])->pack (-side => 'left');
  $frame->Entry ()->pack (-side => 'right');

}

1;

package Tk::glGribPath;

use Tk;
use Tk::FileSelect;

use base qw (Tk::glGrib_Entry);
use strict;

Construct Tk::Widget 'glGribPath';

sub ClassInit 
{
  my ($class, $mw) = @_;
  $class->SUPER::ClassInit ($mw);
}

sub Populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};
  
  my $opts = $self->{glGrib}{opts};
  my $name = $self->{glGrib}{name};

  $self->SUPER::Populate ($args);
  
  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  $frame->Label (-text => $opts->[2])->pack (-side => 'left');

  $self->{glGrib}{entry} = 
  $frame->Entry (-textvariable => \$self->{glGrib}{path})
    ->pack (-side => 'right');

  $frame->Button (-text => 'Browse', 
                  -command => sub { $self->selectPath (); })
    ->pack (-side => 'right');


}

sub selectPath
{
  my $self = shift;

  my $select = $self->FileSelect (-directory => '.');

  my $path = $select->Show ();

  $self->{glGrib}{path} = $path;

}

package Tk::glGribINTEGER;

use Tk;

use base qw (Tk::glGrib_Entry);
use strict;

Construct Tk::Widget 'glGribINTEGER';

package Tk::glGribSTRING;

use Tk;

use base qw (Tk::glGrib_Entry);
use strict;

Construct Tk::Widget 'glGribSTRING';

package Tk::glGribLISTOFSTRINGS;

use Tk;

use base qw (Tk::glGrib_Entry);
use strict;

Construct Tk::Widget 'glGribLISTOFSTRINGS';

package Tk::glGribLISTOFCOLORS;

use Tk;

use base qw (Tk::glGrib_Entry);
use strict;

Construct Tk::Widget 'glGribLISTOFCOLORS';

package Tk::glGribLISTOFFLOATS;

use Tk;

use base qw (Tk::glGrib_Entry);
use strict;

Construct Tk::Widget 'glGribLISTOFFLOATS';

package Tk::glGribFLOAT;

use Tk;

use base qw (Tk::glGrib_Entry);
use strict;

Construct Tk::Widget 'glGribFLOAT';

package Tk::glGribCOLOR;

use Tk;

use base qw (Tk::glGrib_Entry);
use strict;

Construct Tk::Widget 'glGribCOLOR';

package Tk::glGribBOOLEAN;

use Tk;

use base qw (Tk::Frame);
use strict;

Construct Tk::Widget 'glGribBOOLEAN';

sub ClassInit 
{
  my ($class, $mw) = @_;
  $class->SUPER::ClassInit ($mw);
}

sub Populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};
  
  my $opts = $self->{glGrib}{opts};
  my $name = $self->{glGrib}{name};

  $self->SUPER::Populate ($args);
  
  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  $frame->Label (-text => $opts->[2])->pack (-side => 'left');

  $self->{glGrib}{button} = 
  $frame->Checkbutton ()->pack (-side => 'right');

}

sub setCommand
{
  my ($self, $command) = @_;
  $self->{glGrib}{button}
    ->configure (-command => $command);
}

sub setVariable
{
  my ($self, $ref) = @_;
  $self->{glGrib}{button}
    ->configure (-variable => $ref);
}

1;



package Tk::glGrib_Panel;

use Tk;

use base qw (Tk::MainWindow);
use strict;

Construct Tk::Widget 'glGrib_Panel';

sub ClassInit 
{
  my ($class, $mw) = @_;
  $class->SUPER::ClassInit ($mw);
}

sub Populate 
{
  my ($self, $args) = @_;

  $self->{glGrib} = delete $args->{glGrib};
  
  my $opts = $self->{glGrib}{opts};

  $self->SUPER::Populate ($args);

  my $v = $self->Frame ()->pack (-side => 'left', -fill => 'y');
  
  my $frame = $self->Scrolled ('Frame', -scrollbars => 'e', -sticky => 'nswe')->pack (-expand => 1, -fill => 'both');
  $frame = $frame->Frame ()->pack (-expand => 1, -fill => 'both', -side => 'top');

  my (@sep, $on);
  if (@$opts && ($opts->[0] eq 'on'))
    {
      $on = my $w = &TkglGrib::create ($frame, 'on', $opts->[1]);
      $w->setVariable (\$self->{glGrib}{on});
      $w->setCommand (sub { $self->{glGrib}{on} 
                          ? $self->Enable () 
                          : $self->Disable (); });
      $w->pack (-side => 'top', -fill => 'both', -side => 'top')
        unless ($w->isa ('Tk::MainWindow'));
      push @sep,
      $w->Separator ()->pack (-side => 'top', -fill => 'x');
    }

  $frame = $self->{glGrib}{frame} = $frame->Frame ();

  my @opts = @$opts;

  while (my ($key, $opt) = splice (@opts, 0, 2))
    {
      next if ($key eq 'on');
      my $w = &TkglGrib::create ($frame, $key, $opt);
      $w->pack (-side => 'top', -fill => 'both', -side => 'top')
        unless ($w->isa ('Tk::MainWindow'));
      push @sep,
      $w->Separator ()->pack (-side => 'top', -fill => 'x');
    }

  $self->Enable () if ($self->{glGrib}{on});

  my $b = 
  $self->Button (-relief => 'raised', -text => 'Apply', 
                 -command => sub { })
  ->pack (-side => 'left', -fill => 'both');
  $self->Button (-relief => 'raised', -text => 'Close', 
                 -command => sub { $self->destroy (); })
  ->pack (-side => 'right', -fill => 'both');

  my $h = $self->Frame ()->pack (-side => 'top', -fill => 'x');

  $self->ConfigSpecs
  (
    -background => [[@sep], qw/background Background black/],
    -width  => [[$h], qw//],
    -height => [[$v], qw//],
  );

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

package Tk::glGribGrid;

use base qw (Tk::glGrib_Panel);

package Tk::glGribField;

use base qw (Tk::MainWindow);
use strict;

Construct Tk::Widget 'glGribField';

sub ClassInit 
{
  my ($class, $mw) = @_;
  $class->SUPER::ClassInit ($mw);
}

sub Populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};

  $self->SUPER::Populate ($args);

  my @type = qw (scalar vector contour stream isofill);

  my $opts = $self->{glGrib}{opts};
  
  my $n = $self->{glGrib}{notebook} = $self->NoteBook ()->pack (-expand => 1, -fill => 'both', -side => 'top');
  my $g = $self->{glGrib}{general} = $n->add ('general', -label => 'General');
  $g = $g->Scrolled ('Frame', -scrollbars => 'e', -sticky => 'nswe')->pack (-side => 'top', -expand => 1, -fill => 'both');

  my @opts = @$opts;

  my @sep;

  while (my ($key, $opt) = splice (@opts, 0, 2))
    {
      next if (grep { $_ eq $key } @type);
      next if ($key eq 'type');
      my $w = &TkglGrib::create ($g, $key, $opt);
      $w->pack (-side => 'top', -fill => 'both')
        unless ($w->isa ('Tk::MainWindow'));
      push @sep,
      $w->Separator ()->pack (-side => 'top', -fill => 'x');
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

      my $w = &TkglGrib::create ($p, $type, $opt);
      $w->pack (-side => 'top', -fill => 'both')
        unless ($w->isa ('Tk::MainWindow'));

    }
  
  $self->Button (-relief => 'raised', -text => 'Apply', 
                 -command => sub { $self->Apply () })
  ->pack (-side => 'left', -fill => 'both');
  $self->Button (-relief => 'raised', -text => 'Close', 
                 -command => sub { $self->destroy (); })
  ->pack (-side => 'right', -fill => 'both');

  $self->ConfigSpecs 
  (
    -background => [[@sep], qw/background Background black/]
  );

}

sub Apply
{
  my $self = shift;
  print $self->{glGrib}{notebook}->raised (), "\n";
}

1;

package Tk::glGribMainWindow;

use Tk;
use base qw (Tk::MainWindow);
use strict;

Construct Tk::Widget 'glGribMainWindow';

sub ClassInit 
{
  my ($class, $mw) = @_;
  $class->SUPER::ClassInit ($mw);
}

sub Populate 
{
  my ($self, $args) = @_;
  
  $self->SUPER::Populate ($args);
  
  $self->{glGrib}{base} = my $base = &TkglGrib::base ('--');

  $self->{glGrib}{panels} = {};
  
  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');
  
  my @b;
  
  my @base = @$base;
  while (my ($name, $opts) = splice (@base, 0, 2))
    {
      push @b,
      $frame->Button 
       (-text => ucfirst ($name), 
        -command => sub { $self->createPanel ($name); })
        ->pack (-expand => 1, -fill => 'both', -side => 'top');
    }
  
  push @b,
  $frame->Button (-relief => 'raised', -text => 'Quit', 
                  -command => sub { $self->quit (); })
  ->pack (-side => 'top', -fill => 'x');


  $self->ConfigSpecs
  (
    -width => [[@b[-1]], qw//],
  );


}

sub createPanel
{
  my ($self, $name) = @_;
  my $p = $self->{glGrib}{panels}{$name};
  
  return if ($p && &Exists ($p));

  my $opts;

  for (my $i = 0; $i < scalar (@{ $self->{glGrib}{base} }); $i += 2)
    {
      if ($self->{glGrib}{base}[$i] eq $name)
        {
          $opts = $self->{glGrib}{base}[$i+1];
          last;
        }
    }

  my $w = &TkglGrib::create 
     ($self, $name, $opts, 
     'glGrib_Panel', -width => 400, 
     -height => 600);

# die unless ($w->isa ('Tk::glGrib_Panel'));

  $self->{glGrib}{panels}{$name} = $w;
}

sub quit
{
  my $self = shift;

  for my $p (values (%{ $self->{glGrib}{panels} }))
    {
      next unless (&Exists ($p));
      $p->destroy ();
    }

  $self->destroy ();
}

1;


package main;

use strict;
use warnings;

use Tk;
use Tk::NoteBook;
use Tk::Pane;
use FileHandle;
use Data::Dumper;
use JSON;
use FindBin qw ($Bin);
$ENV{GLGRIB_PREFIX} = "$Bin/../..";

use Test::More tests => 1;
BEGIN { use_ok('glGrib') };

my $base;

sub rotate
{
  my ($opt) = 'glGrib'->get ('--scene.rotate_earth.on');
  ($opt =~ s/\.on$/.off/o) or ($opt =~ s/\.off$/.on/o);
  'glGrib'->set ($opt);
}

sub move
{
  'glGrib'->set (qw (--window.position.x 0 --window.position.y 0));
}

sub Tk::Separator
{
  my ($self, %args) = @_;
  my $direction = delete $args{'-orient'} // 'horizontal';
  $self->Frame (-bg => 'black', $direction eq 'vertical' ? '-width' : '-height' => 2, %args);
}

sub enter
{
# print "@_\n";
# print "enter\n";
}

sub leave
{
# print "@_\n";
# print "leave\n";
}

sub debug
{
  my $json = 'glGrib'->json ('++', '--');
  my $h = &decode_json ($json);
  print &Dumper ($h);
  print &Dumper (['glGrib'->window ()]);
}

#'glGrib'->start ('--grid.on', '--landscape.on');

my $main = 'Tk::glGribMainWindow'->new ();

$main->geometry ('+0+0');

$main->bind ('<Leave>' => \&leave);
$main->bind ('<Enter>' => \&enter);

&MainLoop ();

#'glGrib'->stop ();

