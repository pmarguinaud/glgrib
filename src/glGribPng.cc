#include <png.h>


#include <iostream>
#include <exception>

#include "glGribPng.h"

namespace glGrib
{

void ReadPng (const std::string & filename, int * pwidth, int * pheight, 
              BufferPtr<unsigned char> & pixels)
{
  int width, height;
  png_byte color_type;
  png_byte bit_depth;

  png_structp png = png_create_read_struct (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (! png) 
    abort();

  png_infop info = png_create_info_struct (png);
  if (! info) 
    abort();

  if (setjmp (png_jmpbuf (png))) 
    abort();

  FILE * fp = fopen (filename.c_str (), "r");

  if (fp == nullptr)
    throw std::runtime_error (std::string ("Cannot open file :") + filename);

  png_init_io (png, fp);

  png_read_info (png, info);

  width      = png_get_image_width  (png, info);
  height     = png_get_image_height (png, info);
  color_type = png_get_color_type   (png, info);
  bit_depth  = png_get_bit_depth    (png, info);

  // Read any color_type into 8bit depth, RGBA format.
  // See http://www.libpng.org/pub/png/libpng-manual.txt

  if(bit_depth == 16)
    png_set_strip_16 (png);

  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb (png);

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8 (png);

  if (png_get_valid (png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha (png);

  // These color_type don't have an alpha channel then fill it with 0xff.
  if(color_type == PNG_COLOR_TYPE_RGB  ||
     color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler (png, 0xFF, PNG_FILLER_AFTER);

  if(color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb (png);

  png_read_update_info (png, info);

  size_t rowsize = png_get_rowbytes (png, info);
  Buffer<png_bytep> png_rows (height);
  Buffer<png_byte> png_bytes (rowsize * height);

  for(int j = 0; j < height; j++) 
    png_rows[j] = &png_bytes[0] + j * rowsize;

  png_read_image (png, &png_rows[0]);

  fclose (fp);

  pixels = BufferPtr<unsigned char>(width * height * 3);

  for (int j = 0; j < height; j++)
    for (int i = 0; i < width; i++)
      {
        pixels[3*(width*j+i)+0] = png_rows[height-j-1][4*i+0];
        pixels[3*(width*j+i)+1] = png_rows[height-j-1][4*i+1];
        pixels[3*(width*j+i)+2] = png_rows[height-j-1][4*i+2];
      }

  png_destroy_read_struct (&png, &info, nullptr);

  *pwidth = width;
  *pheight = height;
}

void WritePng (const std::string & filename, int width, int height, 
               const BufferPtr<unsigned char> & pixels) 
{
  const size_t format_nchannels = 3;
  size_t nvals = format_nchannels * width * height;

  Buffer<png_byte> png_bytes (nvals);
  Buffer<png_bytep> png_rows (height);

  for (size_t i = 0; i < nvals; i++)
    png_bytes[i] = pixels[i];

  for (int i = 0; i < height; i++)
     png_rows[height-i-1] = &png_bytes[i*width*format_nchannels];

  png_structp png = png_create_write_struct 
                      (PNG_LIBPNG_VER_STRING, 
                       nullptr, nullptr, nullptr);

  if (! png)  
    abort ();

  png_infop info = png_create_info_struct (png);

  if (! info) 
    abort ();

  if (setjmp (png_jmpbuf (png))) 
    abort ();

  FILE * fp = fopen (filename.c_str (), "w");
  if (fp == nullptr)
    throw std::runtime_error (std::string ("Cannot open file :") + filename);


  png_init_io (png, fp);

  png_set_IHDR (png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_write_info (png, info);
  png_write_image (png, &png_rows[0]);
  png_write_end (png, nullptr);
  png_destroy_write_struct (&png, &info);

  fclose (fp);
}

}
