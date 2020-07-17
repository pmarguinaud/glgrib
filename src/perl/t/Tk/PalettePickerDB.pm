package Tk::PalettePickerDB;

use tkbase qw (Tk::Toplevel);

use DBI;
use Tk;
use Tk::Pane;
use POSIX qw (floor ceil);
use Image::Magick;
use Carp qw (croak);
use File::Path;

use strict;

sub slurp
{
  my $f = shift;
  return do { my $fh = 'FileHandle'->new ("<$f"); local $/ = undef; <$fh> };
}

sub getPaletteGif
{
  my $test  = sub { &croak ($_[0]) if ($_[0]); };

  my ($widget, $name, $hexa) = @_;

  my $dir = "$ENV{HOME}/.glGribrc/palettes";

  &mkpath ($dir);
  my $f = "$dir/$name.gif";

  if ((! -f $f) && ($hexa))
    {
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

          $test->($img->SetPixel (x => $i, y => 0,color => [$r, $g, $b]));
        }

      $test->($img->Write ($f));
    }
  
  if (-f $f)
    {
      my $image = $widget->Photo (-data => &slurp ($f));
      my $image20 = $widget->Photo ();
      $image20->copy ($image, -zoom => (1, 20));
      return $image20;
    }
}

sub populate
{
  my ($self, $args) = @_;

  $self->withdraw ();

  my $db = delete $args->{'-db'};
  my $palette = delete $args->{'-palette'};

  my $dbh = 'DBI'->connect ("DBI:SQLite:$db", '', '', {RaiseError => 1}) 
   or die ($DBI::errstr);
  $dbh->{RaiseError} = 1;
  my $get = $dbh->prepare ("SELECT name, hexa FROM palettes ORDER BY name;");

  my $ftop = $self->Scrolled ('Frame', -scrollbars => 'w', -height => 400)
    ->pack (-side => 'top', -expand => 1, -fill => 'both');

  $ftop->bind ('<MouseWheel>', [sub { $_[0]->yview('scroll',-($_[1]/120)*3,'units') }, &Tk::Ev ("D")]);

  $get->execute ();
  $get->bind_columns (\my ($name, $hexa));

  my ($col, $row) = (0, 0);

  my $frame = $ftop->Frame ()->pack (-side => 'top', -anchor => 'w');


  $get->execute ();
  $get->bind_columns (\my ($name, $hexa));
  
  
  while ($get->fetch ())
    {
      my $image20 = &getPaletteGif ($self, $name, $hexa);
      my $p = $name;
  
      $ftop
        ->Button (-image => $image20, -command => sub { $self->{palette} = $p; })
        ->pack (-side => 'top', -expand => 1, -fill => 'x');
    }

  $self
    ->Button (-text => 'Exit', 
              -command => sub { $self->{palette} = $palette; })
    ->pack (-expand => 1, -fill => 'x');
}

sub Show 
{
  my $self = shift ;
  
  $self->Popup ();
  
  $self->focus ();
  $self->waitVariable (\$self->{palette}) ;
  $self->withdraw ();

  return $self->{palette};
}

1;
