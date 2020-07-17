package Tk::glGribFIELDREF;

use Tk;
use Tk::FileSelect;

use tkbase qw (Tk::Frame);
use tkbaselist;
use strict;

sub populate 
{
  my ($self, $args) = @_;
  
  my $frame = $self->Frame ()->pack (-expand => 1, -fill => 'both');

  $self->{glGrib} = delete $args->{glGrib};
  
  if (my $opts = $self->{glGrib}{opts})
    {
      $frame->Label (-text => $opts->[2])->pack (-side => 'top'); 
      $self->{variable} = \$opts->[3];
    }
  else
    {
      $self->{variable} = delete $args->{variable};
    }

  $frame->Button (-text => 'File', -width => 6,
                  -command => sub { $self->selectPath (); })
    ->pack (-side => 'left');

  $frame->Button (-text => 'Field', -width => 6,
                  -command => sub { $self->selectField (); })
    ->pack (-side => 'left');

  $frame->Entry (-textvariable => $self->{variable}, -width => 60)
    ->pack (-side => 'left', -expand => 1, -fill => 'x');

}

sub selectField
{
  my ($self, $ref) = @_;
  
  my $file = ${ $self->{variable} };
  $file =~ s/%.*$//o;
# $file = '/home/phi001/3d/glgrib/share/data/joachim_surf.grib';

  if ($ref)
    {
      ${ $self->{variable} } = "$file%$ref";
      return;
    }


  my @r = 'glGrib'->list ($file);

  if (@r < 20)
    {
      my ($x, $y) = $self->pointerxy ();
     
      my $menu = $self->Menu
      (
        -tearoff => 0, 
        -menuitems =>  
        [
          map 
          { 
            my $ref = $_;
            [command => $ref, -command => sub { $self->selectField ($ref) }]  
          } @r
        ],
      );
     
      $menu->post ($x, $y);
    }
  else
    {
      use Tk::FieldRefPicker;

      my $select = $self->FieldRefPicker (-refs => \@r);

      my $ref = $select->Show ();
      
      $self->selectField ($ref);
    }

}

sub selectPath
{
  my ($self) = @_;

  my $select = $self->FileSelect (-directory => '.');

  my $path = $select->Show ();

  ${$self->getVariable ()} = $path;
}

sub getVariable
{
  my $self = shift;
  return $self->{variable};
}

sub set
{
  my ($self, $value) = @_;
  ${$self->getVariable ()} = $value;
}

1;
