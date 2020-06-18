#!/usr/bin/perl -w

use DBI;
use Tk;
use Tk::Pane;

use FileHandle;
use File::Basename;
use POSIX qw (floor ceil);
use FindBin qw ($Bin);
use Image::Magick;
use Carp qw (croak);


sub test { croak $_[0] if ($_[0]); }

sub getPaletteGif
{
  my ($name, $hexa) = @_;

  mkdir ('palettes');
  my $f = "palettes/$name.gif";

  return $f if (-f $f);

  my $img = 'Image::Magick'->new ();
  $img->Set (size => '256x1');
  $img->ReadImage ('canvas:white');

  my @hexa = map { substr ($_, 0, 6) } 
             grep { length ($_) > 0 } 
             split (m/(........)/o, $hexa);

  my @c = map { m/^(..)(..)(..)$/o; 
               [hex ($1) / 255.0, hex ($2) / 255.0, hex ($3) / 255.0] 
              } @hexa;

  for (my $i = 0; $i < 256; $i++)
    {
      my $j0 = 1 + ($i + 0) * ($#c - 1) / 256;
      my $j1 = 1 + ($i + 1) * ($#c - 1) / 256;
      my $w1 = $j0 - int ($j0); 
      my $w0 = 1 - $w1;
      $j0 = &floor ($j0);
      $j1 = &ceil ($j1);

      my ($r, $g, $b) = ($c[$j0][0] * $w0 + $c[$j1][0] * $w1,
                         $c[$j0][1] * $w0 + $c[$j1][1] * $w1,
                         $c[$j0][2] * $w0 + $c[$j1][2] * $w1);

      &test ($img->SetPixel (x => $i, y => 0,color => [$r, $g, $b]));
    }

  &test ($img->Write ($f));

  return $f;
}


sub slurp
{
  my $f = shift;
  return do { my $fh = 'FileHandle'->new ("<$f"); local $/ = undef; <$fh> };
}

my $dbh = 'DBI'->connect ("DBI:SQLite:$Bin/share/glGrib.db", '', '', {RaiseError => 1}) 
 or die ($DBI::errstr);
$dbh->{RaiseError} = 1;
my $get = $dbh->prepare ("SELECT name, hexa  FROM palettes;");

my $top = 'Tk::MainWindow'->new ();

my $ftop = $top->Scrolled ('Frame')->pack (-side => 'top', -expand => 1, -fill => 'both');

$ftop->bind ('<MouseWheel>', [sub { $_[0]->yview('scroll',-($_[1]/120)*3,'units') }, &Tk::Ev ("D")]);

$get->execute ();
$get->bind_columns (\my ($name, $hexa));


while ($get->fetch ())
  {
    my $f = &getPaletteGif ($name, $hexa);

    my $frame = $ftop->Frame ()->pack (-side => 'top', -anchor => 'w');
    my $image = $frame->Photo (-data => &slurp ($f));
    
    my $image20 = $frame->Photo ();
    $image20->copy ($image, -zoom => (1, 20));
    
    $frame
      ->Button (-image => $image20)
      ->pack (-side => 'left');
    $frame->Label (-text => $name)->pack (-side => 'right');
  }

$get = undef;
$dbh = undef;

$top
  ->Button (-text => 'Exit', 
            -command => sub { exit })
  ->pack ();



&MainLoop ();
