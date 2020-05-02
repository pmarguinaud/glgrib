use strict;
use warnings;

use Tk;
use Data::Dumper;

use Test::More tests => 1;
BEGIN { use_ok('glGrib') };

sub rotate
{
  'glGrib'->set ('--scene.rotate_earth.on');
}

sub debug
{
  print &Dumper (['glGrib'->get ('--scene.rotate_earth')]);
}

'glGrib'->start ('--grid.on', '--landscape.on');

my $top = 'MainWindow'->new ();
$top->geometry ('+0+0');

$top->Button (-relief => 'raised', -text => 'Rotate', -command => sub { &rotate (); })
    ->pack (-side => 'top', -fill => 'x', -expand => 1); 

$top->Button (-relief => 'raised', -text => 'Quit', -command => sub { $top->destroy (); })
    ->pack (-side => 'top', -fill => 'x', -expand => 1);

$top->Button (-relief => 'raised', -text => 'Debug', -command => sub { &debug (); })
    ->pack (-side => 'top', -fill => 'x', -expand => 1);

&MainLoop ();

'glGrib'->stop ();

