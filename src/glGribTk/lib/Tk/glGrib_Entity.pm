package Tk::glGrib_Entity;

use strict;
use Tk;


sub getOptions
{
  my $self = shift;
  &Tk::glGrib::json2tree 
    (&JSON::decode_json ('glGrib'->json (@_, "--$self->{glGrib}{name}")));
}

sub Apply
{
  my $self = shift; 

  my $opts = $self->getOptions ();
  my @diff = &Tk::glGrib::diffOptions 
               ($opts, $self->{glGrib}{opts});
  'glGrib'->set (@diff);
  $self->Reload ();
  $self->saveOpts ();
}

sub Reload
{
  my $self = shift;
  my $opts = $self->getOptions ();
  my @diff = &Tk::glGrib::diffOptions ($self->{glGrib}{opts}, $opts);
  $self->setOptions (@diff);
}

sub setOptions
{
  my $self = shift;
  my @opts = @_;

  while (my $opt = shift (@opts))
    {
      my $w = &Tk::glGrib::getWidgetByOption ($opt);
      my $val;

      if ($opt =~ s/\.(on|off)$/.on/o)
        {
          $w = &Tk::glGrib::getWidgetByOption ($opt);
          $val = $1 eq 'on';
        }
      elsif (&Tk::glGrib::isList ($w))
        {
          $val = [];
          while (@opts && (substr ($opts[0], 0, 2) ne '--'))
            {
              push @$val, shift (@opts);
            }
        }
      else
        {
          $val = shift (@opts);
        }

      if ($w)
        {
          $w->set ($val);
        }
      else
        {
          die &Data::Dumper::Dumper ([$opt, $val]);
        }
    }

}

sub Clear
{
  my $self = shift;
  my $opts = $self->getOptions ('+base');
  my @diff = &Tk::glGrib::diffOptions ($self->{glGrib}{opts}, $opts);
  $self->setOptions (@diff);
}

sub saveOpts
{
  my $self = shift;
  $self->{glGrib}{oldOpts} = &Storable::dclone ($self->{glGrib}{opts});
}

sub Text_
{
  my $self = shift;
  my $opts = $self->getOptions ('+base');
  my @diff = &Tk::glGrib::diffOptions ($opts, $self->{glGrib}{opts});

  use Tk::TextOptions;
  'Tk::TextOptions'->new (-opts => \@diff, -command => sub
     { 'glGrib'->set (@_); $self->Reload () if (&Exists ($self)); },
     -title => "$self->{glGrib}{name} options");
}

sub createButtons
{
  my $self = shift;
  $self->Button (-relief => 'raised', -text => 'Apply', -width => 12,
                 -command => sub { $self->Apply (); })
  ->pack (-side => 'left', -expand => 1, -fill => 'x');
  $self->Button (-relief => 'raised', -text => 'Text', -width => 12,
                 -command => sub { $self->Text_ (); })
  ->pack (-side => 'left', -expand => 1, -fill => 'x');
  $self->Button (-relief => 'raised', -text => 'Reload', -width => 12,
                 -command => sub { $self->Reload (); })
  ->pack (-side => 'left', -expand => 1, -fill => 'x');
  $self->Button (-relief => 'raised', -text => 'Clear', -width => 12,
                 -command => sub { $self->Clear (); })
  ->pack (-side => 'left', -expand => 1, -fill => 'x');
  $self->Button (-relief => 'raised', -text => 'Close', -width => 12,
                 -command => sub { $self->destroy (); })
  ->pack (-side => 'left', -expand => 1, -fill => 'x');
}


1;
