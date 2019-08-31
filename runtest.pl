#!/usr/bin/perl -w
#
use strict;
use File::Path;

my $auto = $ENV{AUTO};
my $comp = $ENV{COMP};

(my $test = shift) =~ s/^test_//o;

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


system ('gdb', '-ex=set confirm on', '-ex=run', '-ex=quit', '--args', @args)
  and die;

if ($auto)
  {
    &mkpath ("test.run/$test");
    
    for my $png (<TEST*.png>)
      {
        my $new = "test.run/$test/$png";
        rename ($png, $new);
	my $ref = "test.ref/$test/$png";
        if ((-f $ref) && ($comp))
          {
            system ('compare', '-metric', 'MAE', $ref, $new, '/dev/null');
	    print "\n"
	  }
      }

  }
    
    
    
