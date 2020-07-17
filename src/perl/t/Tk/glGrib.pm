package Tk::glGrib;

use strict;
use JSON;
use Storable;
use Data::Dumper;

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
  
  $o1 = $o1->[3];
  $o2 = $o2->[3];

  if (ref ($o1))
    {
      return "@$o1" eq "@$o2";
    }
  else
    {
      return $o1 eq $o2;
    }
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
# my $json = do { my $fh = 'FileHandle'->new ('<glgrib.json'); local $/ = undef; <$fh> };
# return &json2tree (&decode_json ($json));
  my $tree = &json2tree (&decode_json ('glGrib'->json ('+base', $_[0])));
  'FileHandle'->new ('>glgrib.pl')->print (&Data::Dumper::Dumper ($tree));
  return $tree;
}

sub exists 
{
  my $class = shift;
  no strict 'refs';
  return scalar (@{"$class\::ISA"});
}

sub isMainWindow
{
  my $class = shift;
  return $class->isa ('Tk::MainWindow');
}

my %map;


sub getWidgetByOpts
{
  my ($opt) = @_;
  my $w = $map{$opt};
  unless (&Exists ($w))
    {
      delete $map{$opt};
    }
  return $w;
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

  my $main = &isMainWindow ("Tk::$class");

  my $w = $main
       ? "Tk::$class"->new (glGrib => {name => $name, opts => $opts}, 
                            -title => ucfirst ($name), @args)
       : $win->$class (glGrib => {name => $name, opts => $opts}, @args);

  # Record widget

  if ($term)
    {
      $map{$opts->[0]} = $w;
    }

  return $w;
}

1;
