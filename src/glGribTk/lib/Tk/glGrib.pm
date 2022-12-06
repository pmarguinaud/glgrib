package Tk::glGrib;

our $VERSION = 1.0;

use strict;
use JSON;
use Storable;
use Data::Dumper;
use FileHandle;
use Tk;
use Tk::Pane;

use Tk::glGrib_Frame;
use Tk::glGrib_Entry;
use Tk::glGrib_Popup;

use Tk::glGribField;

use Tk::glGribBOOLEAN;
use Tk::glGribCOLOR;
use Tk::glGribDATE;
use Tk::glGribFIELDREF;
use Tk::glGribFIELDTYPE;
use Tk::glGribFLOAT;
use Tk::glGribANGLE;
use Tk::glGribINTEGER;
use Tk::glGribLATITUDE;
use Tk::glGribLIST;
use Tk::glGribLONGITUDE;
use Tk::glGribPALETTENAME;
use Tk::glGribPATH;
use Tk::glGribPROJECTION;
use Tk::glGribSCALE;
use Tk::glGribSTRING;
use Tk::glGribTRANSFORMATION;

sub h1
{
  my ($win, $text) = @_;
  my $lab =
  $win->Label (-text => $text, -relief => 'groove')
    ->pack (-fill => 'both', -side => 'top', -ipady => 5);

  my $font = $lab->cget ('-font');
  $font =~ s{(\d+)}{int (2.0 * $1)}eo;
  $lab->configure (-font => $font);
  
  return $lab;
}

sub h2
{
  my ($win, $text) = @_;
  my $lab =
  $win->Label (-text => $text)
    ->pack (-fill => 'both', -side => 'top', -ipady => 5);

  my $font = $lab->cget ('-font');
  $font =~ s{(\d+)}{int (1.5 * $1)}eo;
  $lab->configure (-font => $font);
  
  return $lab;
}

sub optionsToList
{
  my $opts = shift;

  if ((scalar (@$opts) == 4) 
   && (! ref ($opts->[0]))
   && (! ref ($opts->[1]))
   && (! ref ($opts->[2])))
    {
      return $opts;
    }

  my @list;
  my @opts = @$opts;
  
  my $bloc = ($opts[0] =~ m/^--/o) && ($opts[1] eq 'BLOCK');
  if ($bloc)
    {
      shift (@opts) for (1 .. 4);
    }

  while (my ($key, $val) = splice (@opts, 0, 2))
    {
      push @list, &optionsToList ($val);
    }

  return @list;
}

sub eqOptions
{
  my ($o1, $o2) = @_;
  
  die &Data::Dumper::Dumper ([$o1, $o2]) 
    unless ($o1->[0] eq $o2->[0]);
  die &Data::Dumper::Dumper ([$o1, $o2]) 
    unless ($o1->[1] eq $o2->[1]);

  my $type = $o1->[1];
  $type =~ s/[^A-Z]//go;

  my $v1 = $o1->[3];
  my $v2 = $o2->[3];

  my $class = "Tk::glGrib$type";

  return $class->eq ($v1, $v2);
}

sub optionToString
{
  my $o = shift;
  if (ref ($o->[3]))
    {
      return ($o->[0], @{$o->[3]});
    }
  elsif ($o->[0] =~ m/\.on$/o)
    {
      my $opt = $o->[0];
      $opt =~ s/\.on$/.off/o unless ($o->[3]);
      return ($opt);
    }
  else
    {
      return ($o->[0], $o->[3]);
    }
}

sub diffOptions
{
  my ($opts1, $opts2) = @_;

  my @o1 = &optionsToList ($opts1);
  my @o2 = &optionsToList ($opts2);

  my @diff;

  for my $i (0 .. $#o1)
    {
      next if (&eqOptions ($o1[$i], $o2[$i]));
      push @diff, &optionToString ($o2[$i]);
    }

  return @diff;
}

sub isList
{
  my $w = shift;
  return $w->isa ('Tk::glGribLIST');
}

sub json2tree
{
  my $list = shift;

  my $v = [];

  for (@$list)  
    {
      my ($name, $type, $desc, $vals) = @$_;

      $name =~ s/^--//o;
      my @name = split (m/\./o, $name);
      
      if (ref ($vals) =~ m/Boolean/o)
        {
          $vals = $vals ? 1 : 0;
        } 

      my $c = \$v;
      for my $n (@name)
        {
          unless ((scalar (@{${$c}}) > 0) && (${$c}->[-2] eq $n))
            {
              push @{${$c}}, ($n, []);
            }
          $c = \${$c}->[-1];
        }

      $$c = ["--$name", $type, $desc, $vals];

    }

  return $v;
}

sub base
{
  my $tree = &json2tree (&decode_json ('glGrib'->json ('+base', $_[0])));
# 'FileHandle'->new ('>glgrib.pl')->print (&Data::Dumper::Dumper ($tree));
  return $tree;
}

sub exists 
{
  my $class = shift;
  no strict 'refs';
  return scalar (@{"$class\::ISA"});
}

sub isToplevel
{
  my $class = shift;
  return $class->isa ('Tk::Toplevel');
}

my %map;


sub getWidgetByOption
{
  my ($opt) = @_;
  return unless (my $w = $map{$opt});
  unless (&Exists ($w))
    {
      delete $map{$opt};
    }
  return $map{$opt};
}

my @top;

sub createToplevel
{
  my $class = shift;
  my $top = $class->new (@_);

  @top = grep { &Exists ($_) } @top;
  push @top, $top;

  return $top;
}

sub destroyToplevel
{
  for my $top (@top)
    {
      next unless (&Exists ($top));
      $top->destroy ();
    }
}

sub create
{
  my ($win, $name, $opts, $default, @args) = @_;

  my $class = 'glGrib' . ucfirst ($name);
  $class =~ s/[^a-z]+$//io;
  $class =~ s{_(\w)}{uc ($1)}egoms;

  # terminal option

  my $term = ($opts->[0] =~ m/^--/o) && ($opts->[1] ne 'BLOCK');

  if ($term)
    {
      my $type = $opts->[1];
      $type =~ s/[^A-Z]//go;
      $default = "glGrib$type" if (&exists ("Tk::glGrib$type"));
    }

  $class = $default 
    unless (&exists ("Tk::$class"));

  $class ||= 'glGrib_Frame';

  my $main = &isToplevel ("Tk::$class");

  my $w = $main
       ? &createToplevel ("Tk::$class", glGrib => {name => $name, opts => $opts}, 
                          -title => ucfirst ($name), @args)
       : $win->$class (glGrib => {name => $name, opts => $opts}, @args);

  # Record widget

  if ($term)
    {
      $map{$opts->[0]} = $w;
    }

  return $w;
}

sub Tk::Separator
{
  my ($self, %args) = @_;
  my $direction = delete $args{'-orient'} // 'horizontal';
  my $width = delete $args{'-width'};
  $self->Frame (-bg => 'black', $direction eq 'vertical' ? '-width' : '-height' => $width, %args);
}

1;


__END__

# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Tk::glGrib - Perl/Tk interface to glGrib

=head1 SYNOPSIS

=head1 DESCRIPTION

=head1 SEE ALSO

=head1 AUTHOR

Philippe Marguinaud, pmarguinaud@hotmail.com

=cut



