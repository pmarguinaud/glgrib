#!/usr/bin/perl -w

use strict;
use FileHandle;
use File::Find;
use File::Path;
use Data::Dumper;
use Cwd;
use File::Temp;
use File::Basename;


sub slurp
{
  my $f = shift;
  my $fh = 'FileHandle'->new ("<$f");
  return '' unless ($fh);
  local $/ = undef; my $data = <$fh>;
  return $data;
}

my ($changelog) = &slurp ("debian/changelog");

# glgrib (1.0-1)

my ($version) = ($changelog =~ m/glgrib\s+\((\S+)-\d+\)/goms);

my ($options) = &slurp ("debian/source/options");

my ($diff_ignore) = ($options =~ m/diff-ignore\s*=\s*"(\S+)"/goms);

$diff_ignore = qr/$diff_ignore/o;

my @f;

&find ({wanted => sub { my $f = $File::Find::name; push @f, $f unless ($f =~ $diff_ignore); }, no_chdir => 1}, '.');

my %f = map { ($_, 1) } @f;

for my $f (@f)
  {
    my @d = split (m,/,o, $f);
    pop (@d);
    while (@d)
      {
        delete $f{join ('/', @d)};
        pop (@d);
      }
  }

@f = sort keys (%f);

my $dir = &basename (&cwd ());

for (@f)
  {
    $_ = 'File::Spec'->catfile ($dir, $_);
  }


chdir ('..');

'FileHandle'->new (">$dir.list")->print (join ("\n", @f, ""));

my @cmd = ('tar', '--verbatim-files-from', "--files-from=$dir.list", -zcvf => "${dir}_$version.orig.tar.gz");

system (@cmd)
  and die ("Command `@cmd' failed\n");
  




