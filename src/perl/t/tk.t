use strict;
use warnings;

use Tk;
use Data::Dumper;
use JSON;
use FindBin qw ($Bin);
$ENV{GLGRIB_PREFIX} = "$Bin/../..";

use Test::More tests => 1;
BEGIN { use_ok('glGrib') };

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
  print "@_\n";
  print "enter\n";
}

sub leave
{
  print "@_\n";
  print "leave\n";
}

sub debug
{
  my $json = 'glGrib'->json ('++', '--');
  my $h = &decode_json ($json);
  print &Dumper ($h);
  print &Dumper (['glGrib'->window ()]);
}

#'glGrib'->start ('--grid.on', '--landscape.on');

my $top = 'MainWindow'->new ();
$top->geometry ('+0+0');

$top->bind ('<Leave>' => \&leave);
$top->bind ('<Enter>' => \&enter);


$top->Button (-relief => 'raised', -text => 'Rotate', -command => sub { &rotate (); })
    ->pack (-side => 'top', -fill => 'x', -expand => 1); 

$top->Button (-relief => 'raised', -text => 'Quit', -command => sub { $top->destroy (); })
    ->pack (-side => 'top', -fill => 'x', -expand => 1);

$top->Button (-relief => 'raised', -text => 'Debug', -command => sub { &debug (); })
    ->pack (-side => 'top', -fill => 'x', -expand => 1);

$top->Button (-relief => 'raised', -text => 'Move', -command => sub { &move (); })
    ->pack (-side => 'top', -fill => 'x', -expand => 1);

&MainLoop ();

#'glGrib'->stop ();

