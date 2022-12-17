use 5.016000;
use ExtUtils::MakeMaker;
use Cwd;


my $cwd = &cwd ();
my $TOP = "$cwd/../..";


my $prefix = $ENV{GLGRIB_PREFIX} || $TOP;
my $libdir = $ENV{GLGRIB_LIBDIR} || $TOP;

my $USE_EGL = 0;

my @lib = $USE_EGL ? qw (-lEGL -lgbm) : qw (-lGLEW -lglfw);
my @def = $USE_EGL ? '-DUSE_EGL' : '-DUSE_GLFW';
my $suf = $USE_EGL ? "-egl" : "-glfw";

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
  INC               => "-I. -I$TOP/include", 
  OBJECT            => 'glGrib.o',
  LIBS              => "-L$libdir/lib -Wl,-rpath,$libdir/lib " .
                       "-lglGrib$suf -lLFI -lGL " . join (' ', @lib) . ' -lpng -lreadline -lncurses ' .
                       '-ltinfo -lssl -lcrypto -lpthread -lsqlite3 -lcurl ' .
                       '-lshp -leccodes -lnetcdf_c++4',
);
