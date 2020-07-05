#include "glGribBitmap.h"

#include <stdexcept>
#include <fstream>
#include <iostream>

namespace glGrib
{

namespace
{

int S4 (unsigned char * h)
{
  return h[0] + 256 * (h[1] + 256 * (h[2] + 256 * h[3]));
}
  
}
  
void Bitmap (const std::string & file, 
             BufferPtr<unsigned char> & rgb, 
             int * width, int * height)
{
  unsigned char h[54];
  std::ifstream fh (file, std::ios::in | std::ifstream::binary);

  if (! fh)
    throw std::runtime_error (std::string ("Cannot open BMP file : ") + file);

  if (! fh.read (reinterpret_cast<char*> (&h[0]), sizeof (h)))
    std::runtime_error (std::string ("Cannot read BMP file : ") + file);

  int ioff = S4 (&h[10]); 
  int ncol = S4 (&h[18]);
  int nrow = S4 (&h[22]);

  rgb = BufferPtr<unsigned char> (3 * ncol * nrow);
  
  if (! fh.seekg (ioff))
    throw std::runtime_error (std::string ("Cannot seek BMP file : ") + file);

  if (! fh.read (reinterpret_cast<char*> (&rgb[0]), 3 * ncol * nrow))
    throw std::runtime_error (std::string ("Cannot read BMP file : ") + file);


#pragma omp parallel for
  for (int i = 0; i < ncol * nrow; i++)
    {
      unsigned char r = rgb[3*i+2];
      unsigned char g = rgb[3*i+1];
      unsigned char b = rgb[3*i+0];
      rgb[3*i+0] = r;
      rgb[3*i+1] = g;
      rgb[3*i+2] = b;
    }
  
  *width = ncol;
  *height = nrow;
}
  
}
