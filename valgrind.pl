#!/usr/bin/perl -w
#
use strict;
use FileHandle;
use Data::Dumper;

=pod

valgrind --num-callers=100 --show-leak-kinds=all --leak-check=full  --track-origins=yes \
./glgrib.x --landscape.geometry  t49/Z.grb  --field[0].scale 1.01  --field[0].path t49/SFX.CLAY.grb \
> valgrind.eo 2>&1

=cut

sub parse_rm
{
  my $rm = shift;

  my @x = split (m/\n/o, $rm);


  for (@x)
    {
      s/.*\(//o;
      s/\).*//o;
    }

  return join ("|", @x);
}

sub parse_iw
{
  my $iw = shift;

  my @x = split (m/\n/o, $iw);


  for (@x)
    {
      s/.*\(//o;
      s/\).*//o;
    }

  return join ("|", @x);
}

my $file = 'valgrind.eo';
my $text = do { my $fh = 'FileHandle'->new ("<$file"); local $/ = undef; <$fh> };

for ($text)
  {
    s/^==\d+== //gom;
  }


my @iw = ($text =~ m/Invalid write of size \d+(.*?)Address/goms);
my @ir = ($text =~ m/Invalid read of size \d+(.*?)Address/goms);
my @rm = ($text =~ m/(?:\d+,)*\d+ bytes in \d+ blocks are still reachable in loss record \d+ of \d+\s*(at.*?)\n\n/goms);


my %bt;

if(0){

for my $rm (@rm)
  {
    my $bt = &parse_rm ($rm);
    $bt{$bt}++;
  }

}else{

for my $iw (@iw)
  {
    my $bt = &parse_iw ($iw);
    $bt{$bt}++;
  }

}


my @bt = grep { ! m/\bgrib_/o } keys (%bt);

for my $bt (@bt)
  {
    $bt =~ s,\|,\n/,go;
    print $bt, "\n\n";
  }


