#!/usr/bin/perl -w


use strict;
use DBI;

my %colours_;


sub Rgb 
{
  return sprintf ('#%2.2x%2.2x%2.2x', map { 255 * $_ } @_);
}


$colours_{"ecmwf_blue"}       = Rgb(0.25, 0.43, 0.7);
$colours_{"red"}              = Rgb(1.0000, 0.0000, 0.0000);
$colours_{"green"}            = Rgb(0.0000, 1.0000, 0.0000);
$colours_{"blue"}             = Rgb(0.0000, 0.0000, 1.0000);
$colours_{"yellow"}           = Rgb(1.0000, 1.0000, 0.0000);
$colours_{"cyan"}             = Rgb(0.0000, 1.0000, 1.0000);
$colours_{"magenta"}          = Rgb(1.0000, 0.0000, 1.0000);
$colours_{"black"}            = Rgb(0.0000, 0.0000, 0.0000);
$colours_{"avocado"}          = Rgb(0.4225, 0.6500, 0.1950);
$colours_{"beige"}            = Rgb(0.8500, 0.7178, 0.4675);
$colours_{"brick"}            = Rgb(0.6000, 0.0844, 0.0300);
$colours_{"brown"}            = Rgb(0.4078, 0.0643, 0.0000);
$colours_{"burgundy"}         = Rgb(0.5000, 0.0000, 0.1727);
$colours_{"charcoal"}         = Rgb(0.2000, 0.2000, 0.2000);
$colours_{"chestnut"}         = Rgb(0.3200, 0.0112, 0.0000);
$colours_{"coral"}            = Rgb(0.9000, 0.2895, 0.2250);
$colours_{"cream"}            = Rgb(1.0000, 0.8860, 0.6700);
$colours_{"evergreen"}        = Rgb(0.0000, 0.4500, 0.2945);
$colours_{"gold"}             = Rgb(0.7500, 0.5751, 0.0750);
$colours_{"grey"}             = Rgb(0.7000, 0.7000, 0.7000);
$colours_{"khaki"}            = Rgb(0.5800, 0.4798, 0.2900);
$colours_{"kelly_green"}      = Rgb(0.0000, 0.5500, 0.1900);
$colours_{"lavender"}         = Rgb(0.6170, 0.4070, 0.9400);
$colours_{"mustard"}          = Rgb(0.6000, 0.3927, 0.0000);
$colours_{"navy"}             = Rgb(0.0000, 0.0000, 0.4000);
$colours_{"ochre"}            = Rgb(0.6800, 0.4501, 0.0680);
$colours_{"olive"}            = Rgb(0.3012, 0.3765, 0.0000);
$colours_{"peach"}            = Rgb(0.9400, 0.4739, 0.3788);
$colours_{"pink"}             = Rgb(0.9000, 0.3600, 0.4116);
$colours_{"rose"}             = Rgb(0.8000, 0.2400, 0.4335);
$colours_{"rust"}             = Rgb(0.7000, 0.2010, 0.0000);
$colours_{"sky"}              = Rgb(0.4500, 0.6400, 1.0000);
$colours_{"tan"}              = Rgb(0.4000, 0.3309, 0.2000);
$colours_{"tangerine"}        = Rgb(0.8784, 0.4226, 0.0000);
$colours_{"turquoise"}        = Rgb(0.1111, 0.7216, 0.6503);
$colours_{"violet"}           = Rgb(0.4823, 0.0700, 0.7000);
$colours_{"reddish_purple"}   = Rgb(1.0000, 0.0000, 0.8536);
$colours_{"purple_red"}       = Rgb(1.0000, 0.0000, 0.5000);
$colours_{"purplish_red"}     = Rgb(1.0000, 0.0000, 0.2730);
$colours_{"orangish_red"}     = Rgb(1.0000, 0.0381, 0.0000);
$colours_{"red_orange"}       = Rgb(1.0000, 0.1464, 0.0000);
$colours_{"reddish_orange"}   = Rgb(1.0000, 0.3087, 0.0000);
$colours_{"orange"}           = Rgb(1.0000, 0.5000, 0.0000);
$colours_{"yellowish_orange"} = Rgb(1.0000, 0.6913, 0.0000);
$colours_{"orange_yellow"}    = Rgb(1.0000, 0.8536, 0.0000);
$colours_{"orangish_yellow"}  = Rgb(1.0000, 0.9619, 0.0000);
$colours_{"greenish_yellow"}  = Rgb(0.8536, 1.0000, 0.0000);
$colours_{"yellow_green"}     = Rgb(0.5000, 1.0000, 0.0000);
$colours_{"yellowish_green"}  = Rgb(0.1464, 1.0000, 0.0000);
$colours_{"bluish_green"}     = Rgb(0.0000, 1.0000, 0.5000);
$colours_{"blue_green"}       = Rgb(0.0000, 1.0000, 1.0000);
$colours_{"greenish_blue"}    = Rgb(0.0000, 0.5000, 1.0000);
$colours_{"purplish_blue"}    = Rgb(0.1464, 0.0000, 1.0000);
$colours_{"blue_purple"}      = Rgb(0.5000, 0.0000, 1.0000);
$colours_{"bluish_purple"}    = Rgb(0.8536, 0.0000, 1.0000);
$colours_{"purple"}           = Rgb(1.0000, 0.0000, 1.0000);
$colours_{"white"}            = Rgb(1.0000, 1.0000, 1.0000);


use Data::Dumper;

my $dbh = 'DBI'->connect ("dbi:SQLite:dbname=glgrib.db", "", "");

my $exist = $dbh->prepare ('SELECT COUNT (*) FROM COLORS WHERE name = ?');
my $inser = $dbh->prepare ('INSERT INTO COLORS VALUES (?, ?)');


while (my ($col, $hex) = each (%colours_))
  {
    $exist->bind_columns (\my $count);
    $exist->execute ($col);
    $exist->fetchrow ();
    next if ($count);
    $inser->execute ($col, $hex);
  }






