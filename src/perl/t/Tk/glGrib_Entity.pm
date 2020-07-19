package Tk::glGrib_Entity;

use strict;


sub getOptions
{
  my $self = shift;
  &Tk::glGrib::json2tree 
    (&JSON::decode_json ('glGrib'->json (@_, "--$self->{glGrib}{name}")));
}

sub getCurrentOptions
{
  my $self = shift;
  $self->getOptions (); 
}

sub getBasicOptions
{
  my $self = shift;
  $self->getOptions ('+base'); 
}

sub Apply
{
  my $self = shift; 
  my @opts = &Tk::glGrib::diffOptions 
               ($self->{glGrib}{oldOpts}, $self->{glGrib}{opts});
  print &Data::Dumper::Dumper (\@opts);
  'glGrib'->set (@opts);
  $self->saveOpts ();
}

sub Reload
{

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

  $self->saveOpts ();
}

sub saveOpts
{
  my $self = shift;
  $self->{glGrib}{oldOpts} = &Storable::dclone ($self->{glGrib}{opts});
}

sub createButtons
{
  my $self = shift;
  $self->Button (-relief => 'raised', -text => 'Apply', -width => 12,
                 -command => sub { $self->Apply (); })
  ->pack (-side => 'left', -expand => 1, -fill => 'x');
# $self->Button (-relief => 'raised', -text => 'Apply/Close', -width => 12,
#                -command => sub { $self->Apply (); $self->destroy (); })
# ->pack (-side => 'left', -expand => 1, -fill => 'x');
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
