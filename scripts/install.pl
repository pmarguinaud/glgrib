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

sub installPackage
{
  my ($pack, $arch) = @_;

  my @node = do { my $fh = 'FileHandle'->new ("<debian/$pack.install"); <$fh> };
  for (@node)
    {
      chomp;
      s,^usr/,,o;
    }

  my $pwd = &cwd ();

  &rmtree ("$pwd/debian/$pack");
  &mkpath ("$pwd/debian/$pack");

  my @file;

  for my $node (@node)
    {
      &find ({no_chdir => 1, wanted => sub 
      { 
        return unless (-f (my $file = $File::Find::name)); 
        push @file, $file; 
      }}, $node);
    }

  my %map = 
  (
    bin => 'bin',
    share => 'share',
    lib => "lib/$arch",
  );

  for my $file (@file) 
    {
      $file =~ s,^(\w+)/,,o;
      my $dir = $1;
      my $dest = "$pwd/debian/$pack/usr/$map{$dir}";
      print "$dir/$file -> $dest\n";
      chdir ($dir);
      &installFile ($file, $dest);
      chdir ('..');
    }
  
}


my @node = grep { !/^(?:attic|src|debian|scripts|debian\.old)$/o } <*>;

&mkpath ('debian/tmp/usr');


for my $node (@node)
  {
    &runCommand ('cp', '-alf', $node, "debian/tmp/usr/$node");
  }


for (qw(
   usr/CMakeLists.txt.old 
   usr/LICENSE.txt 
   usr/README.md 
   usr/TODO 
   usr/build.sh 
   usr/env.txt 
   usr/lib/libLFI.so 
   usr/lib/libglGrib.so 
   usr/makefile 
   usr/makefile.inc 
))
  {
    unlink ("debian/tmp/$_");
  }



if(0){

my @pack = map { &basename ($_, qw (.install)) } <debian/*.install>;
my @node;

for my $pack (@pack)
  {
    push @node, do { my $fh = 'FileHandle'->new ("<debian/$pack.install"); <$fh> };
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

}


if(0){
my $arch = 'x86_64-linux-gnu';

my @pack = map { &basename ($_, qw (.install)) } <debian/*.install>;

for my $pack (@pack)
  {
    &installPackage ($pack, $arch);
  }
}



