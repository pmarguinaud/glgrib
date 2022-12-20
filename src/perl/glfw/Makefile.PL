package MY;

use strict;

sub const_loadlibs
{
  my $self = shift;

  my $c = $self->SUPER::const_loadlibs (@_);

  my $debian = $ENV{DEB_BUILD_ARCH};

  if ($debian)
    {
      $self->{LD_RUN_PATH} = "";
    }

  return $c;
}

package main;

use 5.016000;
use ExtUtils::MakeMaker;
use Cwd;



my $cwd = &cwd ();
my $TOP = "$cwd/../../..";

my $prefix = $ENV{GLGRIB_PREFIX} || $TOP;
my $libdir = $ENV{GLGRIB_LIBDIR} || $TOP;

my $rpath = "-Wl,-rpath,$libdir/lib";

my $debian = $ENV{DEB_BUILD_ARCH};

if ($debian)
  {
    $rpath = '';
    $prefix = '/usr';
  }

my $USE_EGL = 'glfw' eq 'egl';

my @lib = $USE_EGL ? qw (-lEGL -lgbm) : qw (-lGLEW -lglfw);
my @def = $USE_EGL ? '-DGLGRIB_USE_EGL' : '-DGLGRIB_USE_GLFW';
my $suf = $USE_EGL ? "-egl" : "-glfw";

&WriteMakefile
(
  LD                => 'g++ -std=c++11 -fopenmp',
  CC                => 'g++ -std=c++11 -fopenmp',
  NAME              => 'glGrib::glfw',
  VERSION_FROM      => 'lib/glGrib/glfw.pm', 
  PREREQ_PM         => {}, 
  ABSTRACT_FROM     => 'lib/glGrib/glfw.pm', 
  AUTHOR            => 'Philippe Marguinaud <pmarguinaud@hotmail.com>',
  DEFINE            => "-DGLGRIB_PREFIX='\"$prefix\"' @def", 
  INC               => "-I. -I$TOP/include", 
  OBJECT            => 'glfw.o',
  LIBS              => "-L$libdir/lib $rpath " .
                       "-lglGrib$suf -lLFI -lGL " . join (' ', @lib) . ' -lpng -lreadline -lncurses ' .
                       '-ltinfo -lssl -lcrypto -lpthread -lsqlite3 -lcurl ' .
                       '-lshp -leccodes -lnetcdf_c++4',
  $debian 
?(PREFIX            => "$TOP/debian/tmp/usr") 
:()
);
