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
  ABSTRACT_FROM     => 'lib/Tk/glGrib.pm', 
  AUTHOR            => 'Philippe MARGUINAUD <pmarguinaud@hotmail.com>',
  EXE_FILES         => [ <bin/*> ],
  $debian 
?(PREFIX            => "$TOP/debian/tmp/usr",
  INSTALLDIRS       => 'vendor',
  INSTALLSCRIPT     => "$TOP/debian/tmp/usr/bin") 
:()
);
