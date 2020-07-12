
use FindBin qw ($Bin);
use lib $Bin;

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

my %map;


sub getWidgetByOpts
{
  my ($opt) = @_;
  my $w = $map{$opt};
  unless (&Exists ($w))
    {
      delete $map{$opt};
    }
  return $w;
}

sub create
{
  my ($win, $name, $opts, $default, @args) = @_;


  my $class = 'glGrib' . ucfirst ($name);
  $class =~ s/[^a-z]+$//io;
  $class =~ s{_(\w)}{uc ($1)}egoms;

  # terminal option

  my $term = $opts->[0] =~ m/^--/o;

  if ($term)
    {
      my $type = $opts->[1];
      $type =~ s/[^A-Z]//go;
      $default = "glGrib$type" if (&exists ("Tk::glGrib$type"));
    }

  $class = $default 
    unless (&exists ("Tk::$class"));

  $class ||= 'glGrib_Frame';

  my $w = &isMainWindow ("Tk::$class")
       ? "Tk::$class"->new (glGrib => {name => $name, opts => $opts}, 
                            -title => ucfirst ($name), @args)
       : $win->$class (glGrib => {name => $name, opts => $opts}, @args);
  
  # Record widget

  if ($term)
    {
      $map{$opts->[0]} = $w;
    }

  return $w;
}

package Tk::glGrib_Frame;

use Tk;

use tkbase qw (Tk::Frame);
use strict;

sub populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};

  
  my $opts = $self->{glGrib}{opts};
  my $name = $self->{glGrib}{name};

  my ($on);

  my @opts = @$opts;

  if (@opts && ($opts[0] eq 'on'))
    {
      $on = &TkglGrib::create ($self, 'on', $opts[1]);
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
          my $w = &TkglGrib::create ($frame, $key, $opt);
          $w->pack (-side => 'top', -fill => 'both', -side => 'top');
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

package Tk::glGrib_Entry;

use Tk;

use tkbase qw (Tk::Frame);
use strict;

sub populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};
 
  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  if (my $opts = $self->{glGrib}{opts})
    {
      $frame->Label (-text => $opts->[2])->pack (-side => 'left');
      $self->{variable} = \$opts->[2];
    }
  else
    {
      $self->{variable} = delete $args->{variable};
    }
  

  $self->{entry} =
  $frame->Entry (-textvariable => $self->getVariable (), -validate => 'key',
                 -validatecommand => sub { $self->validate (@_) })
    ->pack (-side => 'right');

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

package Tk::glGribPath;

use Tk;
use Tk::FileSelect;

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


  if ($self->{glGrib}{opts}[1] eq 'STRING-LIST')
    {
      my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');
      $frame->Button (-text => '+', -command => sub { $self->append () })
        ->pack (-side => 'left', -expand => 1, -fill => 'x');
      $frame->Button (-text => '-', -command => sub { $self->remove () })
        ->pack (-side => 'right', -expand => 1, -fill => 'x');
      $self->set ($self->{glGrib}{opts}[3]);
    }
  else
    {
      $self->append ();
    }

}

sub append
{
  my $self = shift;

  my @c = $self->{frame}->children ();
  my $rank = scalar (@c);

  my $frame = $self->{frame}->Frame ()
    ->pack (-side => 'top', -expand => 1, -fill => 'both');

  $frame->Button (-text => 'Browse', 
                  -command => sub { $self->selectPath ($rank); })
    ->pack (-side => 'left');

  if ($self->{glGrib}{opts}[1] eq 'STRING-LIST')
    {
      my $o = $self->{glGrib}{opts}[3];
     
      if (scalar (@c) == scalar (@$o))
        {
          push @$o, '';
        }
      
      $frame->Entry (-textvariable => \$o->[$rank])
        ->pack (-side => 'right', -expand => 1, -fill => 'x');
    }
  else
    {
      $frame->Entry (-textvariable => \$self->{glGrib}{opts}[3])
        ->pack (-side => 'right', -expand => 1, -fill => 'x');
    }

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

sub selectPath
{
  my ($self, $rank) = @_;

  my $select = $self->FileSelect (-directory => '.');

  my $path = $select->Show ();

  my @c = $self->{frame}->children ();
  my $c = $c[$rank];

  my ($e) = ($c->children ())[1];

  my $text = $e->cget ('-textvariable');

  $$text = $path;

}

sub set
{
  my ($self, $value) = @_;

  if ($self->{glGrib}{opts}[1] eq 'STRING-LIST')
    {
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
 else
   {
     $self->{glGrib}{opts}[3] = $value;
   }
}

package Tk::glGribDATE;

use tkbase qw (Tk::glGrib_Entry);

sub validate
{
  return 1;
}

package Tk::glGribINTEGER;

use tkbase qw (Tk::glGrib_Entry);
use tkbaselist;

sub validate
{
  my $self = shift;
  my ($value) = @_;
  return $value =~ m/^\d+$/o;
}

package Tk::glGribSTRING;

use tkbase qw (Tk::glGrib_Entry);
use tkbaselist;

package Tk::glGribFLOAT;

use tkbase qw (Tk::glGrib_Entry);
use tkbaselist;

package Tk::glGribCOLOR;

use tkbase qw (Tk::Frame);
use tkbaselist;
use strict;

use Tk::ColorPicker;

sub populate 
{
  my ($self, $args) = @_;
  
  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  $self->{glGrib} = delete $args->{glGrib};

  if (my $opts = $self->{glGrib}{opts})
    {
      $frame->Label (-text => $opts->[2])->pack (-side => 'left');
      $self->{variable} = \$opts->[3];
    }
  else
    {
      $self->{variable} = delete $args->{variable};
    }

  $self->{entry} =
  $frame->Entry (-textvariable => $self->{variable}, -validate => 'key',
                 -validatecommand => sub { $self->validate (@_); } )
    ->pack (-side => 'right');
  $frame->Button (-text => 'RGB', -command => sub { $self->chooseRGB (); })
    ->pack (-side => 'right');
}

sub getRGB
{
  my $self = shift;
  my $picker = 'Tk::ColorPicker'->new ();
  my $color = $picker->Show ();
  $picker->destroy ();
  return $color;
}

sub chooseRGB
{
  my $self = shift;
  ${ $self->{entry}->cget ('-textvariable') } = $self->getRGB ();
}

sub validate
{
  my $self = shift;
  my ($value) = @_;
  return $value =~ m/^(?:\w*|^#[0-9a-h]*)$/o;
}

sub set
{
  my ($self, $value) = @_;
  ${ $self->{entry}->cget ('-textvariable') } = $value;
}


package Tk::glGribBOOLEAN;

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

  $frame->Label (-text => $opts->[2])->pack (-side => 'left');

  $self->{glGrib}{button} = 
  $frame->Checkbutton (-variable => \$opts->[3])
    ->pack (-side => 'right');

}

sub setCommand
{
  my ($self, $command) = @_;
  $self->{glGrib}{button}
    ->configure (-command => $command);
}

sub getValue
{
  my $self = shift;
  ${ $self->{glGrib}{button}->cget ('-variable') }
}

1;



package Tk::glGrib_Panel;

use Tk;

use tkbase qw (Tk::MainWindow);
use strict;

sub populate 
{
  my ($self, $args) = @_;

  $self->{glGrib} = delete $args->{glGrib};
  
  my $opts = $self->{glGrib}{opts};

  my $frame = $self->Scrolled ('Frame', -width => 400, -height => 400, -scrollbars => 'e', -sticky => 'nswe')->pack (-expand => 1, -fill => 'both');
  $frame = $frame->Frame ()->pack (-expand => 1, -fill => 'both', -side => 'top');

  my (@sep, $on);
  my @opts = @$opts;

  if (@opts && ($opts[0] eq 'on'))
    {
      $on = &TkglGrib::create ($frame, 'on', $opts[1]);
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
      my $w = &TkglGrib::create ($frame, $key, $opt);
      $w->pack (-side => 'top', -fill => 'both', -side => 'top');
      $w->Separator (-width => 2)->pack (-side => 'top', -fill => 'x')
        if (@opts);
    }

  $self->Enable () if ((! $on) || $on->getValue ());

  my $b = 
  $self->Button (-relief => 'raised', -text => 'Apply', 
                 -command => sub { })
  ->pack (-side => 'left', -fill => 'both');
  $self->Button (-relief => 'raised', -text => 'Close', 
                 -command => sub { $self->destroy (); })
  ->pack (-side => 'right', -fill => 'both');

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

package Tk::glGribField;

use tkbase qw (Tk::MainWindow);
use strict;

sub populate 
{
  my ($self, $args) = @_;
  
  $self->{glGrib} = delete $args->{glGrib};

  my @type = qw (scalar vector contour stream isofill);

  my $opts = $self->{glGrib}{opts};
  
  my $n = $self->{glGrib}{notebook} = $self->NoteBook ()->pack (-expand => 1, -fill => 'both', -side => 'top');
  my $g = $self->{glGrib}{general} = $n->add ('general', -label => 'General');
  $g = $g->Scrolled ('Frame', -scrollbars => 'e', -sticky => 'nswe')->pack (-side => 'top', -expand => 1, -fill => 'both');

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

  my $fb = $g->Frame ()->pack (-expand => 1, -fill => 'x');

  for my $type (@type)
    {
      $fb->Radiobutton 
      (
        -text => $type,
        -value => uc ($type),
        -variable => $tt,
        -command => sub { $self->enableTab ($type) },
      )->pack (-side => 'left');
    }

  my @sep;

  while (my ($key, $opt) = splice (@opts, 0, 2))
    {
      next if (grep { $_ eq $key } @type);
      next if ($key eq 'type');
      my $w = &TkglGrib::create ($g, $key, $opt);
      $w->pack (-side => 'top', -fill => 'both')
        unless ($w->isa ('Tk::MainWindow'));
      push @sep,
      $w->Separator (-width => 2)->pack (-side => 'top', -fill => 'x');
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
  print $self->{glGrib}{notebook}->raised (), "\n";
}

1;

package Tk::glGribMainWindow;

use Tk;
use tkbase qw (Tk::MainWindow);
use strict;

sub populate 
{
  my ($self, $args) = @_;

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
     'glGrib_Panel');

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
  my $width = delete $args{'-width'};
  $self->Frame (-bg => 'black', $direction eq 'vertical' ? '-width' : '-height' => $width, %args);
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

