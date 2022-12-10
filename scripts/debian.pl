#!/usr/bin/perl -w

use strict;
use File::Find;
use File::Spec;
use File::Path;
use File::Copy;
use File::Basename;
use FileHandle;
use Cwd;


sub copyFile
{
  my ($src, $dir) = @_;
  my $dst = "$dir/$src";
  &mkpath (&dirname ($dst));
  &copy ($src, $dst)
    or die ("Copy $src, $dst failed");
}

sub copyDirectory
{
  my ($src, $dir) = @_;
  my $dst = "$dir/$src";
  &mkpath (&dirname ($dst));
  my @cmd = ('cp -r', $src, $dst);
  system (@cmd)
    and die ("Command `@cmd' failed");
}

sub createPackageDirectory
{
  my ($pack, $dir) = @_;
  &mkpath ($dir);

  my @list = do { my $fh = 'FileHandle'->new ("<$pack/list"); <$fh> };  
  chomp for (@list);

  for my $f (@list)
    {
      if (-f $f)
        {
          &copyFile ($f, $dir);
        }
      elsif (-d $f)
        {
          &copyDirectory ($f, $dir);
        }
    }
}

(my $version = shift) 
  or die ("Usage: $0 version\n");

my $pwd = &cwd ();
my $top = "$pwd/..";


for my $pack (map { &basename ($_) } <debian/*>)
  {
    my $dir = "$top/$pack-$version";
    &createPackageDirectory ($pack, $dir);
  }


