#!/usr/bin/perl -w


package Tk::ColorPicker;

use strict;
use base qw (Tk::Toplevel);

Construct Tk::Widget 'ColorPicker';

sub Populate
{
  my ($self, $args) = @_;

  my $color = delete $args->{-color};
  print "color=$color\n";

  $self->SUPER::Populate ($args);

  $self->{frame} = $self->Frame (-height => 20)
    ->pack (-side => 'top', -expand => 1, -fill => 'x');


  @{$self}{qw (r g b)} = map { hex ($_) } (($color || '#000000') =~ m/^#([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})$/goms);

  for (qw (r g b))
    {
      $self->Scale (-orient => 'horizontal', -from => 0, -to => 255, -tickinterval => 32, -label => $_,
                    -length => 300, -variable => \$self->{$_}, -command => sub { $self->setColor () })
      ->pack (-expand => 1, -fill => 'both', -side => 'top');
    }

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
  $self->{color} = $self->getColor ();
}

sub setColor
{
  my $self = shift;
  $self->{frame}->configure (-background => $self->getColor ());
}

sub Show 
{
  my $self = shift ;
  
  $self->Popup ();
  
  $self->focus ();
  $self->waitVariable (\$self->{color}) ;
  $self->withdraw ();
  
  return $self->{color};
}



package main;

use strict;
use Tk;
use Data::Dumper;

my $top = 'Tk::MainWindow'->new ();

$top->Button (-text => 'Color', -command => sub { my $cp = $top->ColorPicker (-color => '#00ff00'); print $cp->Show (); })
  ->pack (-side => 'top', -expand => 1, -fill => 'x');
$top->Button (-text => 'Quit', -command => sub { exit (0); })->pack (-side => 'top', -expand => 1, -fill => 'x');

&MainLoop ();

