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

# Files tracked by git only : untracked and ignored files (build products, downloads in
# share/doc/earcut/tools/ext, test.run, local work files) must not go into the source package.
# The contents are taken from the working tree, as before.

my @f = do
{
  local $/ = "\0";
  my $fh = 'FileHandle'->new ("git ls-files -z |") 
    or die ("Cannot run git ls-files\n");
  map { chomp; $_ } <$fh>;
};

@f = grep { ! m/$diff_ignore/o } @f;
@f = grep { (-f $_) || (-l $_) } @f;          # files deleted in the working tree
@f = sort @f;

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
  




