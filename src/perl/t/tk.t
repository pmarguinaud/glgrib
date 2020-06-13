
package glGrib::Tk;

use strict;

sub grid
{
  my $class = shift;
  my ($main, $opts) = @_;
  
  $main->Label (-text => "grid");

}


sub Create
{
  my $class = shift;
  my ($main, $opts) = @_;

  for my $name (sort keys %$opts) 
    {
      if ('glGrib::Tk'->can ($name))
        {
          my $w = 'glGrib::Tk'->$name ($main, $opts->{$name});
          $w->pack (-side => 'top', -fill => 'x', -expand => 1);
        }
      elsif (ref ($opts->{$name}) eq 'HASH')
        {
          my $w = $main->Frame (-label => $name, -borderwidth => 2, -relief => 'groove');
          $w->pack (-side => 'top', -fill => 'x', -expand => 1);
          $class->Create ($w, $opts->{$name});
        }
      elsif (ref ($opts->{$name}) eq 'ARRAY')
        {
          
        }
    }

  return $main;
}

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

sub tree
{
  my $list = shift;

  my $h = {};
  for (@$list)  
    {
      my ($name, $type, $desc, $vals) = @$_;

      $name =~ s/^--//o;
      my @name = split (m/\./o, $name);
      
      my $x = \$h;
      while (my $n = shift (@name))
        {
          ${$x} ||= {};
          $x = \${$x}->{$n};
        }
 
      ${$x} = ["--$name", $type, $desc, $vals];

    }

  return $h;
}

'glGrib'->start ('--grid.on', '--landscape.on');


$base = &tree (&decode_json ('glGrib'->json ('+base', '--')));

my $main = 'MainWindow'->new ();

$main->geometry ('+0+0');

$main->bind ('<Leave>' => \&leave);
$main->bind ('<Enter>' => \&enter);

my $frame = $main->Frame ()->pack (-expand => 1, -fill => 'both');


my $nb;

$nb = $frame->NoteBook ()->pack (-expand => 1, -fill => 'both', -side => 'top');

for my $name (sort keys (%$base))
  {
    next if ($name =~ m/^field/o);
    my $tab = $nb->add ($name, -label => $name);
    my $pan = $tab->Scrolled (qw/Pane -scrollbars osw/)->pack (-expand => 1, -fill => 'both', -side => 'top');
    'glGrib::Tk'->Create ($pan, $base->{$name});
  }

$nb = $frame->NoteBook ()->pack (-expand => 1, -fill => 'both', -side => 'top');
for my $name (sort keys (%$base))
  {
    next unless ($name =~ m/^field/o);
    my $tab = $nb->add ($name, -label => $name);
    my $pan = $tab->Scrolled (qw/Pane -scrollbars osw/)->pack (-expand => 1, -fill => 'both', -side => 'top');
    'glGrib::Tk'->Create ($pan, $base->{$name});
  }


$frame->Button (-relief => 'raised', -text => 'Quit', -command => sub { $main->destroy (); })
    ->pack (-side => 'top', -fill => 'x');


=pod

$frame->Button (-relief => 'raised', -text => 'Rotate', -command => sub { &rotate (); })
    ->pack (-side => 'top', -fill => 'x');

$frame->Button (-relief => 'raised', -text => 'Debug', -command => sub { &debug (); })
    ->pack (-side => 'top', -fill => 'x');

$frame->Button (-relief => 'raised', -text => 'Move', -command => sub { &move (); })
    ->pack (-side => 'top', -fill => 'x');

=cut

&MainLoop ();

'glGrib'->stop ();

