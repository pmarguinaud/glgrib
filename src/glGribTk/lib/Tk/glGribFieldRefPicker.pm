package Tk::FieldRefPicker;

use strict;

use tkbase qw (Tk::Toplevel);


sub populate
{
  my ($self, $args) = @_;

  my $refs = delete $args->{'-refs'};

  $self->withdraw ();

  my $frame = $self->Scrolled ('Frame', -width => 400, -height => 400, -scrollbars => 'e', -sticky => 'nswe')->pack (-expand => 1, -fill => 'both');

  for my $ref (@$refs)
    {
      $frame->Button (-text => $ref, -command => sub { $self->{ref} = $ref })
        ->pack (-expand => 1, -fill => 'x');
    }


}

sub Show 
{
  my $self = shift ;
  
  $self->Popup ();
  
  $self->focus ();
  $self->waitVariable (\$self->{ref}) ;
  $self->withdraw ();
  
  return $self->{ref};
}


1;
