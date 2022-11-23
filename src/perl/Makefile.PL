use 5.024000;
use ExtUtils::MakeMaker;

my $prefix = "$ENV{HOME}/3d/glgrib";

my $USE_EGL = 0;

my @lib = $USE_EGL ? qw (-lEGL -lgbm) : qw (-lGLEW -lglfw);
my @def = $USE_EGL ? '-DUSE_EGL' : '-DUSE_GLFW';

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
  LIBS              => "-L$prefix/lib -Wl,-rpath,$prefix/lib " .
                       '-lglGrib -lLFI -lGL ' . join (' ', @lib) . ' -lpng -lreadline -lncurses ' .
                       '-ltinfo -lssl -lcrypto -lpthread -lsqlite3 -lcurl ' .
                       '-lshp -leccodes -lnetcdf_c++4',
);
