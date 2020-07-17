
use FindBin qw ($Bin);
use lib $Bin;

use Tk::glGrib;
use Tk::glGrib_Frame;
use Tk::glGrib_Popup;
use Tk::glGrib_Entry;
use Tk::glGrib_Panel;

use Tk::glGribPATH;
use Tk::glGribFIELDREF;
use Tk::glGribDATE;
use Tk::glGribINTEGER;
use Tk::glGribPALETTENAME;
use Tk::glGribSTRING;
use Tk::glGribPROJECTION;
use Tk::glGribTRANSFORMATION;
use Tk::glGribFLOAT;
use Tk::glGribSCALE;
use Tk::glGribLONGITUDE;
use Tk::glGribLATITUDE;
use Tk::glGribCOLOR;
use Tk::glGribBOOLEAN;

use Tk::glGribField;
use Tk::glGribMainWindow;

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

'glGrib'->start ();

my $main = 'Tk::glGribMainWindow'->new ();

$main->geometry ('+0+0');

$main->bind ('<Leave>' => \&leave);
$main->bind ('<Enter>' => \&enter);

&MainLoop ();

'glGrib'->stop ();

