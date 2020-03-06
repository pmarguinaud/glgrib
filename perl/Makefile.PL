use 5.026001;
use ExtUtils::MakeMaker;

my $HOME = $ENV{HOME};

my @lib = ("$HOME/3d/glgrib", "$HOME/3d/glgrib/lfi", "$HOME/install/eccodes-2.14.0/lib", "$HOME/3d/usr/lib");

WriteMakefile(
    LD                => 'g++ -std=c++11 -fopenmp',
    CC                => 'g++ -std=c++11 -fopenmp',
    NAME              => 'glGrib',
    VERSION_FROM      => 'lib/glGrib.pm', 
    PREREQ_PM         => {}, 
    ABSTRACT_FROM     => 'lib/glGrib.pm', 
    AUTHOR            => 'Philippe Marguinaud <phi001@giedi-prime>',
    DEFINE            => '', 
    INC               => '-I. -I..', 
    OBJECT            => 'glGribShellPerl.o glGrib.o',
    LIBS              => join (' ', map { "-L$_ -Wl,-rpath,$_" } @lib) .
                         ' -lglGrib -llfi -lGLEW -lGL -lglfw -lpng -lreadline -lncurses ' .
                         '-ltinfo -lssl -lcrypto -lpthread -lsqlite3 -Llfi -llfi -lcurl ' .
                         '-lshp -leccodes',
);
