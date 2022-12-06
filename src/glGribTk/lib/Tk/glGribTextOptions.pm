package Tk::gGribTextOptions;

use strict;

use tkbase qw (Tk::Toplevel);

sub escape
{
  my @t = split (m//o, $_[0]);

  my @s;

  for my $t (@t)
    {   
      push (@s, '\\"')  if ($t eq '"');
      push (@s, '\\\\') if ($t eq '\\');
      push (@s, $t);
    }   

  return join ('', @s);
}

sub populate
{
  my ($self, $args) = @_;

  my $opts = delete $args->{'-opts'};
  my $command = delete $args->{'-command'};

  my $frame = $self->Scrolled 
    ('Frame', -width => 400, -height => 400, 
     -scrollbars => 'e', -sticky => 'nswe')
    ->pack (-side => 'top', -expand => 1, -fill => 'both');

  my $tt = 
  $frame->Text ()
    ->pack (-expand => 1, -fill => 'both');

  for my $i (0 .. $#$opts)
    {
      my $o = $opts->[$i];
      my $oo = $o =~ m/^--/o;
      if ($i)
        {
          $tt->Insert ($oo ? "\n" : " ");
        }
      $oo || ($o = '"' . &escape ($o) . '"');
      $tt->Insert ($o);
    }

  $self->{text} = $tt;
  $self->{command} = $command;

  $frame = $self->Frame ()->pack (-expand => 1, -fill => 'x', -side => 'top');

  $frame->Button (-text => 'Apply', -command => sub { $self->Apply () })
    ->pack (-expand => 1, -fill => 'x', -side => 'left');
  $frame->Button (-text => 'Close', -command => sub { $self->destroy () })
    ->pack (-expand => 1, -fill => 'x', -side => 'left');
}


sub eatSpace
{
  my $text = shift;
  $text =~ s/^\s*//o;
  return $text;
}

sub nextToken
{
  my @c = split (m//o, $_[0]);

  my $t = '';
  my $q;

  while (@c)
    {
      my $c = shift (@c);
      if ($c =~ m/^\s$/o)
        {
          last unless ($q);
        }
      elsif ($c eq '"')
        {
          $q = ! $q;
          next;
        }
      elsif ($c eq '\\')
        {
          $c = shift (@c);
        }
      $t .= $c;
    }

  $_[0] = join ('', @c);

  return $t;
}

sub parse
{
  my $text = shift;
  my @t;
  while (length ($text))
    {
      $text = &eatSpace ($text);
      my $t = &nextToken ($text);
      last unless (length ($t));
      push @t, $t;
    }
  return @t;
}

sub Apply
{
  my $self = shift;

  my @opts = &parse ($self->{text}->Contents ());

  $self->{command}->(@opts)
    if ($self->{command});
}

sub Show 
{
  my $self = shift ;
  $self->Popup ();
  $self->focus ();
}


1;
