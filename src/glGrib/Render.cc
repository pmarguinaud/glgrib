#include "glGrib/Png.h"
#include "glGrib/Render.h"
#include "glGrib/Clear.h"
#include "glGrib/OpenGL.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

namespace glGrib
{

namespace
{

void replace (std::string & fmt, const std::string & K, const std::string & V)
{
  for (int i = 0; ; i++)
    {
      size_t pos = fmt.find (K, i);
      if (pos == std::string::npos)
        break;
      i = pos + V.length ();
      fmt.replace (pos, K.length (), V);
    }
}

void cleanup (std::string & str, char r)
{
  for (size_t i = 0; i < str.size (); i++)
    if ((str[i] == ' ') || (str[i] == '/'))
      str[i] = r;
}

const std::string getOptionsString (Options opts0)
{
  std::vector<std::string> listStr;
  Options opts1;
  OptionsParser p1, p0;

  opts0.traverse ("", &p0);
  opts1.traverse ("", &p1);

  p0.getValue (&listStr, "--", true, false, &p1);

  std::string options;

  for (const auto & str : listStr)
    {
      if (options.size () > 0)
        options += std::string (" ");
      options += str;
    }

  return options;
}

}

Render::Render (const Options & opts)
{
}

void Render::reSize (int w, int h)
{
  opts.width = w;
  opts.height = h;
  auto cg = getContext ();
  glViewport (0, 0, opts.width, opts.height);
  scene.setViewport (opts.width, opts.height);
}

void Render::clear ()
{
  cleared = false;
  scene.clear ();
}

void Render::snapshot (const std::string & format)
{
  const auto & opts = getOptions ();

  BufferPtr<unsigned char> rgb (opts.width * opts.height * 4);

  // glReadPixels does not seem to work well with all combinations of width/height
  // when GL_RGB is used; GL_RGBA on the other hand seems to work well
  // So we get it in RGBA mode and throw away the alpha channel
  glReadPixels (0, 0, opts.width, opts.height, GL_RGBA, GL_UNSIGNED_BYTE, &rgb[0]);

  for (int i = 0; i < opts.width * opts.height; i++)
    for (int j = 0; j < 3; j++)
      rgb[3*i+j] = rgb[4*i+j];


  // %N  -> snapshot_cnt
  // %D  -> date
  // %F  -> Field name

  const OptionDate * date = getScene ().getDate ();
  std::string dstr = date ? date->asString () : "";
  std::string fnam = getScene ().getCurrentFieldName ();

  cleanup (dstr, ':');
  cleanup (fnam, '_');

  std::string fmt = format;

  replace (fmt, "%D", dstr);
  replace (fmt, "%F", fnam);

  std::vector<int> posN;
  for (int i = 0; ; i++)
    {
      size_t pos = fmt.find ("%N", i);
      if (pos == std::string::npos)
        break;
      posN.push_back (pos);
      i = pos + 1;
    }
  
  std::string filename;

  int * snapshot_cnt = &getSnapshotCnt ();

  if (posN.size () > 0)
    {
      while (1)
        {
          struct stat st;
          char cnt[16];
      
          sprintf (cnt, "%4.4d", *snapshot_cnt);
      
          filename = fmt;
      
          for (int i = posN.size () - 1; i >= 0; i--)
            filename.replace (posN[i], 2, std::string (cnt));
      
          if (stat (filename.c_str (), &st) < 0)
            break;
          else
            (*snapshot_cnt)++;
        }
      (*snapshot_cnt)++;
    }
  else
    {
      filename = fmt;
    }

  const std::string options = getOptionsString (getScene ().getOptions ());
  
  WritePng (filename, opts.width, opts.height, rgb, options);
}

void Render::framebuffer (const std::string & format)
{
  const auto & opts = getOptions ();

  if (opts.antialiasing.on)
    {
      unsigned int framebufferMMSA;
      unsigned int texturebufferMMSA;
      unsigned int renderbufferMMSA;
      unsigned int framebufferPOST;
      unsigned int texturebufferPOST;

      glGenFramebuffers (1, &framebufferMMSA);
      glBindFramebuffer (GL_FRAMEBUFFER, framebufferMMSA);

      glGenTextures (1, &texturebufferMMSA);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, texturebufferMMSA);
      glTexImage2DMultisample (GL_TEXTURE_2D_MULTISAMPLE, opts.antialiasing.samples, 
		               GL_RGB, opts.width, opts.height, GL_TRUE);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, 0);
      glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                              GL_TEXTURE_2D_MULTISAMPLE, texturebufferMMSA, 0);

      glGenRenderbuffers (1, &renderbufferMMSA);
      glBindRenderbuffer (GL_RENDERBUFFER, renderbufferMMSA);
      glRenderbufferStorageMultisample (GL_RENDERBUFFER, opts.antialiasing.samples, 
		                        GL_DEPTH24_STENCIL8, opts.width, opts.height);
      glBindRenderbuffer (GL_RENDERBUFFER, 0);
      glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                                 GL_RENDERBUFFER, renderbufferMMSA);

      if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error (std::string ("Framebuffer is not complete"));

      glGenFramebuffers (1, &framebufferPOST);
      glBindFramebuffer (GL_FRAMEBUFFER, framebufferPOST);

      glGenTextures (1, &texturebufferPOST);
      glBindTexture (GL_TEXTURE_2D, texturebufferPOST);
      glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, opts.width, opts.height, 
                    0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                              GL_TEXTURE_2D, texturebufferPOST, 0);	

      if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error (std::string ("Framebuffer is not complete"));

      glBindFramebuffer (GL_FRAMEBUFFER, framebufferMMSA);
   
      getScene ().render ();

      glBindFramebuffer (GL_READ_FRAMEBUFFER, framebufferMMSA);
      glBindFramebuffer (GL_DRAW_FRAMEBUFFER, framebufferPOST);
      glBlitFramebuffer (0, 0, opts.width, opts.height, 0, 0, opts.width, opts.height, 
                         GL_COLOR_BUFFER_BIT, GL_NEAREST);

      glBindFramebuffer (GL_FRAMEBUFFER, framebufferPOST);
   
      snapshot (format);

      glDeleteTextures (1, &texturebufferPOST);
      glDeleteFramebuffers (1, &framebufferPOST);
      glDeleteRenderbuffers (1, &renderbufferMMSA);
      glDeleteTextures (1, &texturebufferMMSA);
      glDeleteFramebuffers (1, &framebufferMMSA);
   
      glBindFramebuffer (GL_FRAMEBUFFER, 0);
    }
  else
    {
      unsigned int framebuffer;
      unsigned int renderbuffer;
      unsigned int texturebuffer;
   
      glGenFramebuffers (1, &framebuffer);
      glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
     
      glGenTextures (1, &texturebuffer);
      glBindTexture (GL_TEXTURE_2D, texturebuffer);
      glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, opts.width, 
                    opts.height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
     
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                              GL_TEXTURE_2D, texturebuffer, 0);
     
      glGenRenderbuffers (1, &renderbuffer);
      glBindRenderbuffer (GL_RENDERBUFFER, renderbuffer);
     
      glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, 
                             opts.width, opts.height); 
      glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                                 GL_RENDERBUFFER, renderbuffer); 
   
      if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error (std::string ("Framebuffer is not complete"));
   
      getScene ().render ();
   
      snapshot (format);
   
      glDeleteRenderbuffers (1, &renderbuffer); 
      glDeleteTextures (1, &texturebuffer);
      glDeleteFramebuffers (1, &framebuffer);

      glBindFramebuffer (GL_FRAMEBUFFER, 0);
    }

}

}
