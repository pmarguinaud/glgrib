use 5.016000;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'glGribTk',
    VERSION_FROM      => 'lib/Tk/glGrib.pm', 
    PREREQ_PM         => {}, 
    ABSTRACT_FROM     => 'lib/Tk/glGrib.pm', 
    AUTHOR            => 'Philippe MARGUINAUD <pmarguinaud@hotmail.com>',
);
