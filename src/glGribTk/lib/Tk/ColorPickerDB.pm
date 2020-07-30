package Tk::ColorPickerDB;

use tkbase qw (Tk::Toplevel);

use DBI;
use Tk;
use Tk::Pane;

sub populate
{
  my ($self, $args) = @_;

  $self->withdraw ();

  my $db = delete $args->{'-db'};

  my $dbh = 'DBI'->connect ("DBI:SQLite:$db", '', '', {RaiseError => 1}) 
   or die ($DBI::errstr);
  $dbh->{RaiseError} = 1;
  my $get = $dbh->prepare ("SELECT name, hexa  FROM colors ORDER by hexa;");

  my $ftop = $self->Scrolled ('Frame', -scrollbars => 'w', -height => 400)
    ->pack (-side => 'top', -expand => 1, -fill => 'both');

  $ftop->bind ('<MouseWheel>', [sub { $_[0]->yview('scroll',-($_[1]/120)*3,'units') }, &Tk::Ev ("D")]);

  $get->execute ();
  $get->bind_columns (\my ($name, $hexa));

  my ($col, $row) = (0, 0);

  my $frame = $ftop->Frame ()->pack (-side => 'top', -anchor => 'w');

  while ($get->fetch ())
    {
      my $color = $hexa;
      $frame
        ->Button (-background => $hexa, -text => substr ($name, 0, 10), -width => 10,
                  -command => sub { $self->{color} = $color; })
        ->grid (-column => $col++, -row => $row);
      if ($col % 10 == 0)
        {
          $col = 0; $row++;
        }
    }

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

1;
