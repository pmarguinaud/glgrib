#!/usr/bin/perl -w
#
use strict;
use File::Path;
use Data::Dumper;

my $auto = $ENV{AUTO};
my $comp = $ENV{COMP};

(my $test = shift (@ARGV)) =~ s/^test_//o;
my $exec = shift (@ARGV);

my @args = @ARGV;

if ($args[0] !~ m/^--/o)
  {
    $auto = 0;
  }

if ($auto)
  {
    my $off = grep { m/^--window.offscreen.on$/o } @args;
    push (@args, '--window.offscreen.on') unless ($off);
    push (@args, '--window.offscreen.format', 'TEST_%N.png');
    for (<TEST*.png>)
      {
        unlink ($_);
      }
  }

my @cmd = ('gdb', '-ex=set confirm on', '-ex=run', '-ex=quit', '--args', $exec, @args);

system (@cmd)
  and die;

if ($auto)
  {
    &mkpath ("test.run/$test");
    
    for my $png (<TEST*.png>)
      {
        my $new = "test.run/$test/$png";
        rename ($png, $new);
	my $ref = "test.ref/$test/$png";
	my $dif = "test.run/$test/diff_$png";
        if ((-f $ref) && ($comp))
          {
            my @cmd = ('compare', '-metric', 'MAE', $ref, $new, $dif);
	    print "@cmd\n";
	    system (@cmd);
	    print "\n"
	  }
      }

  }
    
    
    
