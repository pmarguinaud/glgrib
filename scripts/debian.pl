#!/usr/bin/perl -w

use strict;
use warnings FATAL => 'all';
use File::Find;
use File::Spec;
use File::Path;
use File::Copy;
use File::Basename;
use FileHandle;
use Data::Dumper;
use Getopt::Long;
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

sub getVersionFromChangeLog
{
  my $changelog = shift;
  my $text = do { my $fh = 'FileHandle'->new ("<$changelog"); <$fh> };
  my ($version) = ($text =~ m/^\S+\s+\((\S+)\)\s+/o);
  $version =~ s/-\d+$//o;
  return $version;
}

sub getPackageFromChangeLog
{
  my $changelog = shift;
  my $text = do { my $fh = 'FileHandle'->new ("<$changelog"); <$fh> };
  my ($package) = ($text =~ m/^(\S+)\s+/o);
  return $package;
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

sub createPackage
{
  my ($opts, $pack, $top) = @_;

  my $pwd = &cwd ();

  my $version = &getVersionFromChangeLog ("debian/$pack/changelog");
  my $dir = "$top/$pack-$version";

  if ($opts->{'create-directory'})
    {
     
      &rmtree ($dir) if (-d $dir);
      &mkpath ($dir) or die ("Cannot create path $dir");
     
      my @list = do { my $fh = 'FileHandle'->new ("<debian/$pack/list"); <$fh> };  
      chomp for (@list);
     
      for my $f (@list)
        {
          &copyNode ($f, $dir);
        }
     
      &runCommand ('cp', '-alf', "debian/$pack", "$dir/debian");
    }

  if ($opts->{'create-archive'})
    {
      &runCommand ('tar', -C => $top, 
                   '-zcvf' => "$top/${pack}_$version.orig.tar.gz", 
                   &basename ($dir));
    }

  if ($opts->{'compile'})
    {
      chdir ($dir);
      &runCommand (qw (debuild -us -uc));
      chdir ($pwd);
    }

}

sub installFile
{
  my $dest = shift;

  return unless (-f (my $src = $File::Find::name));
  
  my $dst = 'File::Spec'->catfile ($dest, $src);

  $dst = 'File::Spec'->rel2abs ($dst);

  &mkpath (&dirname ($dst));
  &copyNode ($src, &dirname ($dst));
}

sub installPackage
{
  my $opts = shift;
  my $pack = &getPackageFromChangeLog ('debian/changelog');

  my $pwd = &cwd ();

  (my $arch = $opts->{arch})
    or die ("Missing --arch option");

  for my $bin (<bin/*>)
    {
      &runCommand (qw (patchelf --remove-rpath), $bin);
    }

  for my $lib (grep { -l } <lib/*>)
    {
      unlink ($lib);
    }

  my %map = 
  (
    bin => 'bin',
    share => 'share',
    lib => "lib/$arch",
  );

  for my $dir (sort keys (%map))
    {
      next unless (-d $dir);
      my $dest = "$pwd/debian/$pack/usr/$map{$dir}";
      chdir ($dir);
      &find ({wanted => sub { &installFile ($dest, @_) }, no_chdir => 1}, '.');
      chdir ('..');
    }
}

my $pwd = &cwd ();
my $top = "$pwd/..";

my %opts;
my @opts_f = qw (create-archive create-directory install compile help);
my @opts_s = qw (arch);

&GetOptions
(
  map ({ ($_, \$opts{$_}) } @opts_f),
  map ({ ("$_=s", \$opts{$_}) } @opts_s),
);

if ($opts{help})
  {
    print &basename ($0) . ":\n";
    for my $opt (@opts_f)
      {
        print "  --$opt\n";
      }
  }

if ($opts{install})
  {
    &installPackage (\%opts);
  }
else
  {
    for my $pack (map { &basename ($_) } <debian/*>)
      {
        &createPackage (\%opts, $pack, $top);
        last;
      }
  }


