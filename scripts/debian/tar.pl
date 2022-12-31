#!/usr/bin/perl -w

use warnings FATAL => 'all';
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

my ($changelog) = &slurp ("packaging/debian/changelog");

my ($version) = ($changelog =~ m/glgrib\s+\((\S+)-\d+\)/goms);

my ($options) = &slurp ("packaging/debian/source/options");

my ($diff_ignore) = ($options =~ m/diff-ignore\s*=\s*"(\S+)"/goms);

$diff_ignore = qr/$diff_ignore/o;

my @f;

&find ({wanted => sub 
{ 
  my $f = $File::Find::name; 
  return if ($f =~ $diff_ignore); 
  return if ($f =~ m/\.(?:o|so|so\.\d+)$/o);
  return if ($f =~ m,bin/glgrib-(?:glfw|egl),o);
  return if ($f =~ m,blib,o);
  push @f, $f; 
}, no_chdir => 1}, '.');

my %f = map { ($_, 1) } @f;

# Remove intermediate directories

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
  




