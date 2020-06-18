#!/usr/bin/perl -w


package Tk::ColorPicker;

use strict;

use POSIX qw (fmod);

use base qw (Tk::Toplevel);

Construct Tk::Widget 'ColorPicker';

sub min { return $_[0] < $_[1] ? $_[0] : $_[1]; }
sub max { return $_[0] > $_[1] ? $_[0] : $_[1]; }

sub rgb2hsv 
{
  my ($r, $g, $b) = map { $_ / 255.0 } @_;

  my ($h, $s, $v) = (0.0);
  my $min = &min (&min ($r, $g), $b); 
  my $max = &max (&max ($r, $g), $b); 

  if ($max == $min)
    {
      $h = 0.0;
    }
  elsif ($max == $r)
    {
      $h = &fmod (60.0 * ($g - $b) / ($max - $min) + 360.0, 360.0);
    }
  elsif ($max == $g)
    {
      $h =        60.0 * ($b - $r) / ($max - $min) + 120.0;
    }
  elsif ($max == $b)
    {
      $h =        60.0 * ($r - $g) / ($max - $min) + 240.0;
    }

  if ($max == 0.0)
    {
      $s = 0.0; 
    }
  else
    {
      $s = 1.0 - $min / $max;
    }
  
  $v = $max;

  return map { int ($_) } ($h, 100 * $s, 100 * $v);
}


sub hsv2rgb 
{
  my ($h, $s, $v) = ($_[0], map { $_ / 100. } @_[1..2]);

  my $hi = int ($h / 60.0) % 6;
  my $f = $h / 60.0 - $hi;

  my $l = $v * (1.0 - $s);
  my $m = $v * (1.0 - $f * $s);
  my $n = $v * (1.0 - (1.0 - $f) * $s);

  my ($r, $g, $b) = (0.0, 0.0, 0.0);

  my @select =
  (
    sub { $r = $v; $g = $n; $b = $l; },
    sub { $r = $m; $g = $v; $b = $l; },
    sub { $r = $l; $g = $v; $b = $n; },
    sub { $r = $l; $g = $m; $b = $v; },
    sub { $r = $n; $g = $l; $b = $v; },
    sub { $r = $v; $g = $l; $b = $m; },
  );

  $select[$hi]->();

  return map { int ($_ * 255) } ($r, $g, $b);
}



sub Populate
{
  my ($self, $args) = @_;

  my $color = delete $args->{-color};

  $self->SUPER::Populate ($args);

  $self->withdraw ();

  $self->{frame} = $self->Frame (-height => 20)
    ->pack (-side => 'top', -expand => 1, -fill => 'x');


  @{$self}{qw (r g b)} = map { hex ($_) } (($color || '#000000') 
                         =~ m/^#([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})$/goms);

  for (qw (r g b))
    {
      $self->Scale (-orient => 'horizontal', -from => 0.0, -to => 255, 
                    -tickinterval => 32, -label => uc ($_), -showvalue => 1,
                    -length => 300, -variable => \$self->{$_}, 
                    -command => sub { $self->setColorRGB () })
      ->pack (-expand => 1, -fill => 'both', -side => 'top');
    }

  $self->Scale (-orient => 'horizontal', -from => 0.0, -to => 360, 
                -tickinterval => 36, -label => 'h', -showvalue => 1,
                -length => 300, -variable => \$self->{h}, 
                -command => sub { $self->setColorHSV () })
  ->pack (-expand => 1, -fill => 'both', -side => 'top');

  $self->Scale (-orient => 'horizontal', -from => 0.0, -to => 100, 
                -tickinterval => 36, -label => 's', -showvalue => 1,
                -length => 300, -variable => \$self->{s}, 
                -command => sub { $self->setColorHSV () })
  ->pack (-expand => 1, -fill => 'both', -side => 'top');

  $self->Scale (-orient => 'horizontal', -from => 0.0, -to => 100, 
                -tickinterval => 36, -label => 'v', -showvalue => 1,
                -length => 300, -variable => \$self->{v}, 
                -command => sub { $self->setColorHSV () })
  ->pack (-expand => 1, -fill => 'both', -side => 'top');

  $self->Button (-text => 'OK', -command => sub { $self->OnClick () })
    ->pack (-expand => 1, -fill => 'x', -side => 'top');
}

sub getColor
{
  my $self = shift;
  sprintf ('#%2.2x%2.2x%2.2x', @{$self}{qw (r g b)});
}

sub OnClick
{
  my $self = shift;
  $self->{done}++;
}

sub setColorRGB
{
  my $self = shift;
  @{$self}{qw (h s v)} = &rgb2hsv (@{$self}{qw (r g b)});
  $self->{frame}->configure (-background => $self->getColor ());
}

sub setColorHSV
{
  my $self = shift;
  @{$self}{qw (r g b)} = &hsv2rgb (@{$self}{qw (h s v)});
  $self->{frame}->configure (-background => $self->getColor ());
}

sub Show 
{
  my $self = shift ;
  
  $self->Popup ();
  
  $self->focus ();
  $self->waitVariable (\$self->{done}) ;
  $self->withdraw ();
  
  return $self->getColor ();
}



package main;

use strict;
use Tk;
use Tk::ColorEditor;
use Data::Dumper;

my $top = 'Tk::MainWindow'->new ();

$top->Button (-text => 'Color', -command => sub { my $cp = $top->ColorPicker (-color => '#00ff00'); print $cp->Show (); })
  ->pack (-side => 'top', -expand => 1, -fill => 'x');

$top->Button (-text => 'Color 1', -command => sub { my $couleur = $top->chooseColor( -title => 'Choisir couleur' ); })
  ->pack (-side => 'top', -expand => 1, -fill => 'x');

$top->Button (-text => 'Color 2', -command => sub { my $ed = $top->ColorEditor( -title => 'Choisir couleur' ); $ed->Show (); })
  ->pack (-side => 'top', -expand => 1, -fill => 'x');

$top->Button (-text => 'Quit', -command => sub { exit (0); })->pack (-side => 'top', -expand => 1, -fill => 'x');

&MainLoop ();

