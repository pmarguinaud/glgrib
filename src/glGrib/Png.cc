#include <png.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>

#include <iostream>
#include <exception>

#include "glGrib/Png.h"

namespace glGrib
{

namespace
{

const std::string getUserName ()
{
  std::string User = "unknown";
  uid_t uid = geteuid ();
  struct passwd * pw = getpwuid (uid);

  if (pw)
    User = pw->pw_name;

  return User;
}

const std::string getHostName ()
{
  std::string Host = "unknown";
  const size_t len = 256;
  char host_[len+1];

  if (gethostname (host_, len) == 0)
    Host = host_;

  return Host;
}

const std::string getTime ()
{
  std::string Time = "unknown";
  const size_t len = 256;
  char time_[len+1];
  time_t t = time (nullptr);

  if (strftime (time_, len, "%H:%M:%S %d/%m/%Y", localtime (&t)) > 0)
    Time = time_;

  return Time;
}

}

void ReadPng (const std::string & filename, int * pwidth, int * pheight, 
              BufferPtr<unsigned char> & pixels)
{
  int width, height;
  png_byte color_type;
  png_byte bit_depth;
  std::string mess;

  png_structp png = nullptr;
  png_infop info = nullptr;
  FILE * fp = nullptr;
  
  png = png_create_read_struct (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (! png) 
    {
      mess = "Failed creating PNG handle";
      goto fail;
    }

  info = png_create_info_struct (png);
  if (! info) 
    {
      mess = "Failed creating PNG info handle";
      goto fail;
    }

  if (setjmp (png_jmpbuf (png))) 
    {
      mess = "Failed decoding PNG image";
      goto fail;
    }

  fp = fopen (filename.c_str (), "r");

  if (fp == nullptr)
    {
      mess = "Cannot open file";
      goto fail;
    }

  {
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
   
    if (fclose (fp) != 0)
      {
        mess = "Cannot close file";
        fp = nullptr;
        goto fail;
      }
   
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
  return;
fail:
  if (fp != nullptr)
    fclose (fp);

  if (png != nullptr)
    {
      if (info != nullptr)
        {
          png_destroy_read_struct (&png, &info, nullptr);
	}
      else
        {
          png_destroy_read_struct (&png, nullptr, nullptr);
	}
    }

  throw std::runtime_error (mess + std::string (" : ") + filename);
}

void WritePng (const std::string & filename, int width, int height, 
               const BufferPtr<unsigned char> & pixels, const std::string & options) 
{
  const size_t format_nchannels = 3;
  size_t nvals = format_nchannels * width * height;
  std::string mess;
  png_structp png = nullptr;
  png_infop info = nullptr;
  const int NTEXT = 4, NUSER = 0, NCREATOR = 1, NOPTIONS = 2, NTIME = 3;
  png_text text[NTEXT];
  FILE * fp = nullptr;
  const std::string user = getUserName () + std::string ("@") + getHostName (),
	            time = getTime ();

  text[NUSER].key = (png_charp)"Author";
  text[NUSER].text = (png_charp)user.c_str ();
  text[NUSER].compression = PNG_TEXT_COMPRESSION_NONE;

  text[NCREATOR].key = (png_charp)"Creator";
  text[NCREATOR].text = (png_charp)"glgrib";
  text[NCREATOR].compression = PNG_TEXT_COMPRESSION_NONE;

  text[NOPTIONS].key = (png_charp)"Options";
  text[NOPTIONS].text = (png_charp)options.c_str ();
  text[NOPTIONS].compression = PNG_TEXT_COMPRESSION_NONE;

  text[NTIME].key = (png_charp)"Time";
  text[NTIME].text = (png_charp)time.c_str ();
  text[NTIME].compression = PNG_TEXT_COMPRESSION_NONE;


  Buffer<png_byte> png_bytes (nvals);
  Buffer<png_bytep> png_rows (height);

  for (size_t i = 0; i < nvals; i++)
    png_bytes[i] = pixels[i];

  for (int i = 0; i < height; i++)
     png_rows[height-i-1] = &png_bytes[i*width*format_nchannels];


  png = png_create_write_struct 
          (PNG_LIBPNG_VER_STRING, 
           nullptr, nullptr, nullptr);

  if (! png)  
    {
      mess = "Failed creating PNG handle";
      goto fail;
    }
  
  info = png_create_info_struct (png);

  if (! info) 
    {
      mess = "Failed creating PNG info handle";
      goto fail;
    }

  if (setjmp (png_jmpbuf (png))) 
    {
      mess = "Failed creating PNG image";
      goto fail;
    }

  fp = fopen (filename.c_str (), "w");

  if (fp == nullptr)
    {
      mess = "Cannot open file";
      goto fail;
    }

  png_init_io (png, fp);

  png_set_text (png, info, text, NTEXT);

  png_set_IHDR (png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_write_info (png, info);
  png_write_image (png, &png_rows[0]);
  png_write_end (png, nullptr);
  png_destroy_write_struct (&png, &info);

  if (fclose (fp) != 0)
     {
       mess = "Cannot close file";
     }
  return;
fail:
  if (fp != nullptr)
    fclose (fp);

  if (png != nullptr)
    {
      if (info != nullptr)
        {
          png_destroy_read_struct (&png, &info, nullptr);
	}
      else
        {
          png_destroy_read_struct (&png, nullptr, nullptr);
	}
    }

  throw std::runtime_error (mess + std::string (" : ") + filename);
}

}
