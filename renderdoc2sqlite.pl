#!/usr/bin/perl -w

use strict;
use DBI;
use File::Basename;
use FileHandle;
use Data::Dumper;


sub getline
{
  my $fh = shift;
  my $line = <$fh>;
  return () unless ($line);
  chomp ($line);
  my @line = split (m/\,\s*/o, $line);
  return @line;
}


my $f = shift;
my $b = &basename ($f, qw (.csv));

my $dbh = 'DBI'->connect ("DBI:SQLite:$b.db", '', '', {RaiseError => 1}) 
 or die ($DBI::errstr);
$dbh->{RaiseError} = 1;

my $fh = 'FileHandle'->new ("<$f");

my @h = map { (my $h = $_) =~ s/\W/_/go; $h } &getline ($fh);

$dbh->do ("CREATE TABLE IF NOT EXISTS $b (" . join (', ', map { "$_ REAL" } @h) . ");");

$dbh->do ("BEGIN TRANSACTION;");

my $sth = $dbh->prepare ("INSERT INTO $b VALUES (" . join (', ', map { '?' } @h) . ");");

while (my @line = &getline ($fh)) 
  {
    next if (grep { $_ eq 'Restart' } @line);
    $sth->execute (@line);
  }

$dbh->do ("COMMIT TRANSACTION;");

