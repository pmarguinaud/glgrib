#!/usr/bin/perl -w

use strict;
use warnings FATAL => 'all';
use Data::Dumper;
use File::Basename;
use FileHandle;
use File::Find;
use File::Spec;
use File::Path;
use File::Copy;
use Cwd;

local $SIG{__DIE__} = sub
{
  for (my $i = 0; ; $i++)
    {
      last unless (my @c = caller ($i));
      printf ("%s:%d\n", @c[1,2]);
    }
};

sub runCommand
{
  my @cmd = @_;
  print "@cmd\n";
  system (@cmd)
    and die ("Command `@cmd' failed");
}

sub copyLink
{
  my ($src, $dir) = @_;
  my $dst = "$dir/$src";
  &mkpath (&dirname ($dst));
  symlink (readlink ($src), $dst)
    or die ("Creating symbolic link $dst failed");
}

sub copyFile
{
  my ($src, $dir) = @_;
  my $dst = "$dir/$src";
  &mkpath (&dirname ($dst));
  link ($src, $dst)
    or die ("Copy $src, $dst failed");
}

sub copyDirectory
{
  my ($src, $dir) = @_;
  my $dst = "$dir/$src";
  &mkpath (&dirname ($dst));
  &runCommand ('cp', '-alf', $src, $dst);
}

sub copyNode
{
  my ($f, $dir) = @_;

  if (-l $f)
    {
      &copyLink ($f, $dir);
    }
  elsif (-f $f)
    {
      &copyFile ($f, $dir);
    }
  elsif (-d $f)
    {
      &copyDirectory ($f, $dir);
    }
}

sub installFile
{
  my ($src, $dir) = @_;
  &copyNode ($src, $dir);
}

my @pack = map { &basename ($_, qw (.install)) } <debian/*.install>;
my @node;

for my $pack (@pack)
  {
    push @node, grep { !/^#/o } do { my $fh = 'FileHandle'->new ("<debian/$pack.install"); <$fh> };
  }

for (@node)
  {
    chomp;
    s/\s+.*$//o;
    s,^(\w+/),,o;
  }

for my $node (@node)
  {
    for (glob ($node))
      {
        &installFile ($_, "debian/tmp/usr");
      }
  }


