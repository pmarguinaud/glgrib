#!/usr/bin/perl -w

use strict;
use File::Find;
use File::Basename;
use File::Path;
use File::Spec;
use Cwd;

my $DEST = $ENV{DEST};

die unless ($DEST);

sub install
{
  return unless (-f (my $src = $File::Find::name));
  
  my $dst = 'File::Spec'->catfile ($DEST, $src);

  for ($src, $dst)
    {
      $_ = 'File::Spec'->rel2abs ($_);
    }

  my @command = ('install', $src, $dst);

  &mkpath (&dirname ($dst));

  print "@command\n";
  system (@command) && die ("Command `@command' failed : $!\n");
  
}


&find ({wanted => \&install, no_chdir => 1}, qw (lib bin share));



