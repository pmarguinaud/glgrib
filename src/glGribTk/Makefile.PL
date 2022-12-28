package MY;

use strict;
use Data::Dumper;

sub manifypods
{
  my $self = shift;

  my $r = $self->SUPER::manifypods (@_);

  my $debian = $ENV{DEB_BUILD_ARCH};

  if ($debian)
    {
      $r =~ s/(\s*)$//o;
      my $space = $1;
      $r .= "\n\t\\rm -f \$(INST_MAN1DIR)/*.gz";
      $r .= "\n\t\$(COMPRESS) -f \$(INST_MAN1DIR)/*$space";
    }

  return $r;
}

package main;

use 5.016000;
use ExtUtils::MakeMaker;
use Cwd;

my $cwd = &cwd ();
my $TOP = "$cwd/../..";

my $debian = $ENV{DEB_BUILD_ARCH};

&WriteMakefile
(
  NAME              => 'glGribTk',
  VERSION_FROM      => 'lib/Tk/glGrib.pm', 
  PREREQ_PM         => {'glGrib::gflw' => '0.01', 'Tk' => '804.035'}, 
  ABSTRACT_FROM     => 'bin/glgrib-tk', 
  AUTHOR            => 'Philippe MARGUINAUD <pmarguinaud@hotmail.com>',
  EXE_FILES         => [ <bin/*> ],
  $debian 
?(PREFIX            => "$TOP/debian/tmp/usr",
  INSTALLDIRS       => 'vendor',
  INSTALLSCRIPT     => "$TOP/debian/tmp/usr/bin")
:()
);
