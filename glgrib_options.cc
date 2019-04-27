#include "glgrib_options.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


void glgrib_options::parse (int argc, char * argv[])
{
  int opt = 0;

  const char * const short_opts = "s:Sg:";
  struct option long_options[] = 
  {
      {"width",     required_argument, 0,  'w' },
      {"height",    required_argument, 0,  'h' },
      {"shell",     no_argument,       0,  'S' },
      {"geometry",  required_argument, 0,  'g' },
      {0,           0,                 0,  0   }
  };
  
  int long_index = 0;
  while ((opt = getopt_long (argc, argv, short_opts, 
          long_options, &long_index)) != -1) 
    {
      switch (opt) 
        {
           case 'w' : width  = std::stoi (optarg);     break;
           case 'h' : height = std::stoi (optarg);     break;
           case 'S' : shell  = true;                   break;
           case 'g' : geometry = std::string (optarg); break;
           default: 
               exit (EXIT_FAILURE);
       }
  }


}
