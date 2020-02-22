#include "glgrib_ticks.h"
#include "glgrib_trigonometry.h"
#include "glgrib_program.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

glgrib_ticks & glgrib_ticks::operator= (const glgrib_ticks & other)
{
  if ((this != &other) && other.isReady ())
    {
      clear ();
      opts = other.opts;
      setReady ();
    }
  return *this;
}

void glgrib_ticks::setup (const glgrib_options_ticks & o)
{
  opts = o;
  setReady ();
}

void glgrib_ticks::clear ()
{
  labels.clear ();
  if (isReady ())
    {
      glDeleteVertexArrays (1, &VertexArrayID);
      glDeleteVertexArrays (1, &VertexArrayID_frame);
    }
  width = 0;
  height = 0;
  ready = false;
}

glgrib_ticks::~glgrib_ticks ()
{
  clear (); 
}

void glgrib_ticks::render (const glm::mat4 & MVP) const
{
  if (! ready)
    return;

  if (opts.lines.on)
    {
      glgrib_program * program = glgrib_program::load (glgrib_program::TICKS);
      program->use ();

      int kind = std::min (1, std::max (0, opts.lines.kind));

      program->set ("MVP", MVP);
      program->set ("N", float (glgrib_string::N));
      program->set ("S", float (glgrib_string::S));
      program->set ("W", float (glgrib_string::W));
      program->set ("E", float (glgrib_string::E));
      program->set ("color0", opts.lines.color);
      program->set ("length", opts.lines.length);
      program->set ("width", opts.lines.width);
      program->set ("kind", kind);

      glBindVertexArray (VertexArrayID);

      if (opts.lines.width == 0.0f)
        {
          unsigned int ind[2] = {1, 2};
          glDrawElementsInstanced (GL_LINES, 2, GL_UNSIGNED_INT, ind, numberOfTicks);
        }
      else
        {
          if (kind == 0)
            {
              unsigned int ind[6] = {0, 1, 2, 0, 2, 3};
              glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, numberOfTicks);
            }
          else if (kind == 1)
            {
              unsigned int ind[3] = {0, 1, 2};
              glDrawElementsInstanced (GL_TRIANGLES, 3, GL_UNSIGNED_INT, ind, numberOfTicks);
            }
        }

      glBindVertexArray (0);
    }

  if (opts.frame.on)
    {
      glgrib_program * program = glgrib_program::load (glgrib_program::FTICKS);
      program->use ();


      program->set ("MVP", MVP);
  
      float ratio = float (width) / float (height);

      program->set ("xmin", vopts.clip.xmin * ratio);
      program->set ("xmax", vopts.clip.xmax * ratio);
      program->set ("ymin", vopts.clip.ymin);
      program->set ("ymax", vopts.clip.ymax);
      program->set ("width", opts.frame.width);
      program->set ("color0", opts.frame.color);


      glBindVertexArray (VertexArrayID_frame);
      unsigned int ind[6] = {0, 1, 2, 0, 2, 3};
      glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, 4);
      glBindVertexArray (0);
    }


  if (opts.labels.on)
    labels.render (MVP);

}


void glgrib_ticks::createStr 
(const glgrib_options_ticks_side & sopts,
 glgrib_string::align_t _align, const glgrib_view & view, 
 std::vector<std::string> & S, std::vector<float> & X, 
 std::vector<float> & Y, std::vector<float> & A,
 std::vector<glgrib_string::align_t> & align)
{
  float ratio = float (width) / float (height);

  const int n = 40;
  const int nx = n;
  const int ny = (height * nx) / width;

  class xyllv_t
  {
  public:
    xyllv_t (float _x, float _y, float _lon, float _lat, bool _valid)
           : x (_x), y (_y), lon (_lon), lat (_lat), valid (_valid) {}
    float x, y;
    float lon, lat;
    bool valid;
  };

  std::vector<xyllv_t> xyllv;

  const float dyminf = 0.00f, dymaxf = 0.00f;

  int nxy;

  switch (_align)
    {
      case glgrib_string::E: nxy = ny; break;
      case glgrib_string::W: nxy = ny; break;
      case glgrib_string::N: nxy = nx; break;
      case glgrib_string::S: nxy = nx; break;
    }
         
  // Start at 1, finish at nxy-1 : avoid corners
  for (int i = 1; i < nxy-1; i++)
    {
      float x, y, lon = 0.0f, lat = 0.0f;

      auto cy = [&] () 
      {
         return height * (vopts.clip.ymin + float (i) / float (ny - 1) 
                       * (vopts.clip.ymax - vopts.clip.ymin));
      };

      auto cx = [&] () 
      {
         return width * (vopts.clip.xmin + float (i) / float (nx - 1) 
                      * (vopts.clip.xmax - vopts.clip.xmin));
      };

      switch (_align)
        {
          case glgrib_string::E: 
            x = width * vopts.clip.xmax; y = cy ();                               
	    break;
          case glgrib_string::W: 
	    x = width * vopts.clip.xmin; y = cy ();                               
	    break;
          case glgrib_string::N: 
	    x = cx ();                   y = height * (vopts.clip.ymax - dymaxf); 
	    break;
          case glgrib_string::S: 
	    x = cx ();                   y = height * (vopts.clip.ymin + dyminf); 
	    break;
        }
         
      int c = view.get_latlon_from_screen_coords (x, y, &lat, &lon);
      xyllv.push_back (xyllv_t (x, y, lon, lat, c != 0));

    }



  for (int i = 0; i < xyllv.size ()-1; i++)
    if (xyllv[i+0].valid && xyllv[i+1].valid)
      {
        switch (_align)
          {
            case glgrib_string::E:
            case glgrib_string::W:
              {
                float y0 = xyllv[i+0].y, y1 = xyllv[i+1].y, x = xyllv[i].x;
                float lat0 = xyllv[i+0].lat, lat1 = xyllv[i+1].lat;
                int ilat0 = (lat0 + 90.0f) / sopts.dlat;
                int ilat1 = (lat1 + 90.0f) / sopts.dlat;
                for (int ilat = ilat0; ilat <= ilat1; ilat++)
                  {
                    float lat = ilat * sopts.dlat - 90.0f;
                    if ((lat0 <= lat) && (lat <= lat1))
                      {
                        char tmp[32];
                	float a = (lat - lat0) / (lat1 - lat0);
                	sprintf (tmp, opts.labels.format.c_str (), lat);
                	std::string s (tmp);
			while ((s.length () > 0) && (s[0] == ' '))
                          s = s.substr (1);
                	S.push_back (s);
                	X.push_back (x * ratio / width);
                	Y.push_back ((y0 * (1.0f - a) + y1 * a) / height);
                        A.push_back (0.0f);
			align.push_back (_align);
                      }
                  }
	      }
	    break;
            case glgrib_string::N:
            case glgrib_string::S:
              {
                float x0 = xyllv[i+0].x, x1 = xyllv[i+1].x, y = xyllv[i+0].y;
                float lon0 = xyllv[i+0].lon, lon1 = xyllv[i+1].lon;

                while (lon0 < 0) lon0 += 360.0f;
                while (lon1 < 0) lon1 += 360.0f;

                while (lon1 < lon0)
                  lon1 += 360.0f;

		if ((lon1 - lon0) > fmod (lon0 + 360.0f - lon1, 360.0f))
                  {
                    std::swap (lon0, lon1);
		    std::swap (x0, x1);
		    while (lon1 < lon0)
                      lon1 += 360.0f;
		  }

                int ilon0 = lon0 / sopts.dlon;
                int ilon1 = lon1 / sopts.dlon;
                for (int ilon = ilon0; ilon <= ilon1; ilon++)
                  {
                    float lon = ilon * sopts.dlon;
                    if ((lon0 <= lon) && (lon <= lon1))
                      {
                        char tmp[32];
                	float a = (lon - lon0) / (lon1 - lon0);
                	sprintf (tmp, opts.labels.format.c_str (), fmod (lon + 180.0f, 360.0f) - 180.0f);
                	std::string s (tmp);
                	S.push_back (s);
                	X.push_back ((x0 * (1.0f - a) + x1 * a) / width * ratio);
                	Y.push_back (y / height);
                        A.push_back (0.0f);
			align.push_back (_align);
                      }
                  }
	      }
	    break;
	  }

      }

}


void glgrib_ticks::resize (const glgrib_view & view)
{
  if ((! opts.labels.on) && (! opts.lines.on) && (! opts.frame.on))
    return;

  if (! ready)
    return;

  if ((vopts == view.getOptions ()) && 
      (width == view.getWidth ()) && 
      (height == view.getHeight ()))
    return;

  vopts = view.getOptions ();
  width = view.getWidth (); 
  height = view.getHeight ();

  // Create ticks labels
  std::vector<std::string> S; 
  std::vector<float> X, Y, A;
  std::vector<glgrib_string::align_t> align;

  createStr (opts.E, glgrib_string::E, view, S, X, Y, A, align);
  createStr (opts.W, glgrib_string::W, view, S, X, Y, A, align);
  createStr (opts.N, glgrib_string::N, view, S, X, Y, A, align);
  createStr (opts.S, glgrib_string::S, view, S, X, Y, A, align);

  if (opts.labels.on)
    {
      labels.clear ();
      labels.setShared (false);
      labels.setChange (false);

      glgrib_font_ptr font = new_glgrib_font_ptr (opts.labels.font); 

      labels.setup2D (font, S, X, Y, opts.labels.font.scale, align, A);
      labels.setForegroundColor (opts.labels.font.color.foreground);
      labels.setBackgroundColor (opts.labels.font.color.background);
    }


  if (opts.lines.on)
    {
      // Coordinates of ticks
      std::vector<glm::vec3> XYa (S.size ());

      for (int i = 0; i < XYa.size (); i++)
        {
          XYa[i].x = X[i];
          XYa[i].y = Y[i];
          XYa[i].z = align[i];
        }

      vertexbuffer = new_glgrib_opengl_buffer_ptr (XYa.size () * sizeof (XYa[0]), XYa.data ());

      glGenVertexArrays (1, &VertexArrayID);
      glBindVertexArray (VertexArrayID);
      
      vertexbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (0); 
      glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
      glVertexAttribDivisor (0, 1);

      glBindVertexArray (0);

      numberOfTicks = XYa.size ();
    }

  if (opts.frame.on)
    {
      // Needed to use a shader
      glGenVertexArrays (1, &VertexArrayID_frame);
    }
}



