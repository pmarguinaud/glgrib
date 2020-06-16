#!/usr/bin/perl

# http://perlmonks.org/?node_id=1185809

use strict;
use warnings;
use Tk;

sub Tk::Separator
  {
  my ($self, %rest ) = @_;
  my $direction = delete $rest{'-orient'} // 'horizontal';
  $self->Frame( %{ {%rest, -bg => 'black',
    $direction eq 'vertical' ? '-width' : '-height' => 2 } } );
  }

my $mw = MainWindow->new;

$mw->Label( -text => 'top part', -height => 5,
  )->pack;
$mw->Separator()->pack( -fill => 'x' );

$mw->Label( -text => 'bottom part', -height => 5,
  )->pack( -side => 'bottom');
$mw->Separator()->pack( -side => 'bottom', -fill => 'x' );

$mw->Label( -text => 'left part', -height => 5, -padx => 25,
  )->pack( -side => 'left');

$mw->Separator( -orient => 'vertical')->pack( -side => 'left', -fill => 'y' );

$mw->Label( -text => 'middle part', -height => 5, -padx => 25,
  )->pack( -side => 'left');

$mw->Separator( -orient => 'vertical')->pack( -side => 'left', -fill=> 'y' );

$mw->Label( -text => 'right part', -height => 5, -padx => 25,
  )->pack( -side => 'left');

MainLoop;
