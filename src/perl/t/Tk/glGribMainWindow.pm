package Tk::glGribMainWindow;

use Tk;
use tkbase qw (Tk::MainWindow);
use strict;

use glGrib;

sub populate 
{
  my ($self, $args) = @_;

  'glGrib'->start ();

  $self->{glGrib}{base} = my $base = &Tk::glGrib::base ('--');

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
          $opts =
          &Tk::glGrib::json2tree 
            (&JSON::decode_json ('glGrib'->json ("--$name")));
          $opts = $opts->[1];
	  $self->{glGrib}{base}[$i+1] = $opts;
          last;
        }
    }

  $self->{glGrib}{panels}{$name} = 
    &Tk::glGrib::create ($self, $name, $opts, 'glGrib_Panel');
}

sub quit
{
  my $self = shift;

  &Tk::glGrib::destroyToplevel ();

  $self->destroy ();

  'glGrib'->stop ();

  exit (0);
}

1;



