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


my ($CXXFLAGS, $LDFLAGS) = ('', '');

if ($debian)
  {
    $rpath = '';
    $prefix = '/usr';
    $CXXFLAGS = $ENV{DEB_CXXFLAGS};
    $LDFLAGS = $ENV{DEB_LDFLAGS};
  }

my $USE_EGL = 'glfw' eq 'egl';

my @lib = $USE_EGL ? qw (-lEGL -lgbm) : qw (-lGLEW -lglfw);
my @def = $USE_EGL ? '-DGLGRIB_USE_EGL' : '-DGLGRIB_USE_GLFW';
my $suf = $USE_EGL ? "-egl" : "-glfw";

&WriteMakefile
(
  LD                => "g++ -std=c++11 $LDFLAGS",
  CC                => "g++ -std=c++11 $CXXFLAGS",
  NAME              => 'glGrib::glfw',
  VERSION_FROM      => 'lib/glGrib/glfw.pm', 
  PREREQ_PM         => {}, 
  ABSTRACT_FROM     => 'lib/glGrib/glfw.pm', 
  AUTHOR            => 'Philippe Marguinaud <pmarguinaud@hotmail.com>',
  DEFINE            => "-DGLGRIB_PREFIX='\"$prefix\"' @def", 
  INC               => "-I. -I$TOP/include", 
  OBJECT            => 'glfw.o',
  LIBS              => "-L$libdir/lib $rpath -lglGrib$suf",
  $debian 
?(PREFIX            => "$TOP/debian/tmp/usr",
  INSTALLDIRS       => 'vendor',) 
:()
);
