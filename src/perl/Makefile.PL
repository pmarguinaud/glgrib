use 5.024000;
use ExtUtils::MakeMaker;

my $prefix = "$ENV{HOME}/3d/glgrib";

my $NETCDF_PREFIX  = "/home/marguina/install/netcdf-c-4.7.4";
my $ECCODES_PREFIX = "/home/marguina/install/eccodes-2.14.0";

my $USE_EGL = 0;

my @lib = $USE_EGL ? qw (-lEGL -lgbm) : qw (-lGLEW -lglfw);
my @def = $USE_EGL ? '-DUSE_EGL' : '-DUSE_GLFW';

my @L = ("$prefix/lib", "$NETCDF_PREFIX/lib", "$ECCODES_PREFIX/lib");

&WriteMakefile
(
  LD                => 'g++ -std=c++11 -fopenmp',
  CC                => 'g++ -std=c++11 -fopenmp',
  NAME              => 'glGrib',
  VERSION_FROM      => 'lib/glGrib.pm', 
  PREREQ_PM         => {}, 
  ABSTRACT_FROM     => 'lib/glGrib.pm', 
  AUTHOR            => 'Philippe Marguinaud <pmarguinaud@hotmail.com>',
  DEFINE            => "-DGLGRIB_PREFIX='\"$prefix\"' @def", 
  INC               => '-I. -I..', 
  OBJECT            => 'glGrib.o',
  LIBS              => join (' ', map { "-L$_ -Wl,-rpath,$_" } @L) .
                       ' -lglGrib -lLFI -lGL ' . join (' ', @lib) . ' -lpng -lreadline -lncurses ' .
                       '-ltinfo -lssl -lcrypto -lpthread -lsqlite3 -lcurl ' .
                       '-lshp -leccodes -lnetcdf_c++4',
);
