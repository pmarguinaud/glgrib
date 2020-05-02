#include "version.h"

flat in int rank;

out vec4 color;

uniform vec4 RGBA0[256];
uniform int rank2rgba[256];

void main ()
{
  if(false){
  color.r = float (rank) / 255.;
  color.g = float (rank) / 255.;
  color.b = float (rank) / 255.;
  color.a = float (rank) / 255.;
  }else{
  color = RGBA0[rank2rgba[rank]];
  }
  if(false)
  if(rank2rgba[rank] == 255)
    color = vec4 (1., 1., 1., 1.);
}

