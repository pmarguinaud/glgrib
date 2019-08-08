#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_png.h"

void glgrib_png (const std::string & filename, int width, int height, unsigned char * pixels) 
{
  png_byte * png_bytes = NULL;
  png_byte ** png_rows = NULL;
  const size_t format_nchannels = 3;
  size_t nvals = format_nchannels * width * height;
  FILE * fp = fopen (filename.c_str (), "w");

  png_bytes = (png_byte *)malloc (nvals * sizeof (png_byte));
  png_rows = (png_byte **)malloc (height * sizeof (png_byte *));

  for (int i = 0; i < nvals; i++)
    png_bytes[i] = pixels[i];

  for (int i = 0; i < height; i++)
     png_rows[height-i-1] = &png_bytes[i*width*format_nchannels];

  png_structp png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (! png)  
    abort ();

  png_infop info = png_create_info_struct (png);

  if (! info) 
    abort ();

  if (setjmp (png_jmpbuf (png))) 
    abort ();

  png_init_io (png, fp);

  png_set_IHDR (png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_write_info (png, info);
  png_write_image (png, png_rows);
  png_write_end (png, NULL);
  png_destroy_write_struct (&png, &info);

  fclose (fp);

  free (png_bytes);
  free (png_rows);
}

