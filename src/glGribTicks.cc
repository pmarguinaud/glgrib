#include "glGribTicks.h"
#include "glGribTrigonometry.h"
#include "glGribProgram.h"
#include "glGribClear.h"

#include <cmath>
#include <iostream>
#include <vector>

glGrib::Ticks & glGrib::Ticks::operator= (const glGrib::Ticks & other)
{
  if ((this != &other) && other.isReady ())
    {
      clear (*this);
      opts = other.opts;
      ticks = other.ticks;
      frame = other.frame;
      setReady ();
    }
  return *this;
}

void glGrib::Ticks::setup (const glGrib::OptionsTicks & o)
{
  if (o.lines.on)
    goto on;

  if (o.frame.on)
    goto on;

  if (o.labels.on)
    goto on;

  return;

on:

  opts = o;
  setReady ();
}

template <>
void glGrib::Ticks::ticks_t::render (const glm::mat4 & MVP) const
{
  glGrib::Program * program = glGrib::Program::load ("TICKS");
  program->use ();
  
  int kind = std::min (1, std::max (0, ticks->opts.lines.kind));
  
  program->set ("MVP", MVP);
  program->set ("N", float (glGrib::String::N));
  program->set ("S", float (glGrib::String::S));
  program->set ("W", float (glGrib::String::W));
  program->set ("E", float (glGrib::String::E));
  program->set ("color0", ticks->opts.lines.color);
  program->set ("length", ticks->opts.lines.length);
  program->set ("width", ticks->opts.lines.width);
  program->set ("kind", kind);
  
  if (ticks->opts.lines.width == 0.0f)
    {
      unsigned int ind[2] = {1, 2};
      glDrawElementsInstanced (GL_LINES, 2, GL_UNSIGNED_INT, ind, ticks->numberOfTicks);
    }
  else
    {
      if (kind == 0)
        {
          unsigned int ind[6] = {0, 1, 2, 0, 2, 3};
          glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, ticks->numberOfTicks);
        }
      else if (kind == 1)
        {
          unsigned int ind[3] = {0, 1, 2};
          glDrawElementsInstanced (GL_TRIANGLES, 3, GL_UNSIGNED_INT, ind, ticks->numberOfTicks);
        }
    }
  
}

template <>
void glGrib::Ticks::frame_t::render (const glm::mat4 & MVP) const
{
  glGrib::Program * program = glGrib::Program::load ("FTICKS");
  program->use ();
  
  program->set ("MVP", MVP);
  
  float ratio = float (ticks->width) / float (ticks->height);
  
  program->set ("xmin", ticks->vopts.clip.xmin * ratio);
  program->set ("xmax", ticks->vopts.clip.xmax * ratio);
  program->set ("ymin", ticks->vopts.clip.ymin);
  program->set ("ymax", ticks->vopts.clip.ymax);
  program->set ("width", ticks->opts.frame.width);
  program->set ("color0", ticks->opts.frame.color);
  
  
  unsigned int ind[6] = {0, 1, 2, 0, 2, 3};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, 4);
}

void glGrib::Ticks::render (const glm::mat4 & MVP) const
{
  if (! isReady ())
    return;

  if (opts.lines.on)
    ticks.VAID.render (MVP);

  if (opts.frame.on)
    frame.VAID.render (MVP);

  if (opts.labels.on)
    labels.render (MVP);

}


void glGrib::Ticks::createStr 
(const glGrib::OptionsTicksSide & sopts,
 glGrib::String::align_t _align, const glGrib::View & view, 
 std::vector<std::string> & S, std::vector<float> & X, 
 std::vector<float> & Y, std::vector<float> & A,
 std::vector<glGrib::String::align_t> & align)
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
      case glGrib::String::E: nxy = ny; break;
      case glGrib::String::W: nxy = ny; break;
      case glGrib::String::N: nxy = nx; break;
      case glGrib::String::S: nxy = nx; break;
      default: 
        throw std::runtime_error (std::string ("Unexpected alignment"));
        break;
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
          case glGrib::String::E: 
            x = width * vopts.clip.xmax; y = cy ();                               
	    break;
          case glGrib::String::W: 
	    x = width * vopts.clip.xmin; y = cy ();                               
	    break;
          case glGrib::String::N: 
	    x = cx ();                   y = height * (vopts.clip.ymax - dymaxf); 
	    break;
          case glGrib::String::S: 
	    x = cx ();                   y = height * (vopts.clip.ymin + dyminf); 
	    break;
          default:
            throw std::runtime_error (std::string ("Unexpected alignment"));
            break;
        }
         
      int c = view.getLatLonFromScreenCoords (x, y, &lat, &lon);
      xyllv.push_back (xyllv_t (x, y, lon, lat, c != 0));

    }


  const bool NSWE = sopts.nswe.on;

  for (size_t i = 1; i < xyllv.size (); i++)
    if (xyllv[i-1].valid && xyllv[i+0].valid)
      {
        switch (_align)
          {
            case glGrib::String::E:
            case glGrib::String::W:
              {
                float y0 = xyllv[i-1].y, y1 = xyllv[i+0].y, x = xyllv[i-1].x;
                float lat0 = xyllv[i-1].lat, lat1 = xyllv[i+0].lat;
                int ilat0 = (lat0 + 90.0f) / sopts.dlat;
                int ilat1 = (lat1 + 90.0f) / sopts.dlat;
                for (int ilat = ilat0; ilat <= ilat1; ilat++)
                  {
                    float lat = ilat * sopts.dlat - 90.0f;
                    if ((lat0 <= lat) && (lat <= lat1))
                      {
                        char tmp[32];
                	float a = (lat - lat0) / (lat1 - lat0);
                	std::string s;
			if (NSWE)
                          {
                            std::string L = lat >= 0 ? "N" : "S";
                	    sprintf (tmp, opts.labels.format.c_str (), std::abs (lat));
			    s = std::string (tmp) + L;
			  }
			else
                          {
                	    sprintf (tmp, opts.labels.format.c_str (), lat);
			    s = std::string (tmp);
			  }
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
            case glGrib::String::N:
            case glGrib::String::S:
              {
                float x0 = xyllv[i-1].x, x1 = xyllv[i+0].x, y = xyllv[i-1].y;
                float lon0 = xyllv[i-1].lon, lon1 = xyllv[i+0].lon;

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
                	std::string s;
			lon = fmod (lon + 180.0f, 360.0f) - 180.0f;
			if (NSWE)
                          {
                            std::string L = lon >= 0 ? "E" : "W";
                	    sprintf (tmp, opts.labels.format.c_str (), std::abs (lon));
			    s = std::string (tmp) + L;
			  }
			else
                          {
                	    sprintf (tmp, opts.labels.format.c_str (), lon);
			    s = std::string (tmp);
			  }
                	S.push_back (s);
                	X.push_back ((x0 * (1.0f - a) + x1 * a) / width * ratio);
                	Y.push_back (y / height);
                        A.push_back (0.0f);
			align.push_back (_align);
                      }
                  }
	      }
	    break;
            default:
              break;
	  }
      }

}


void glGrib::Ticks::reSize (const glGrib::View & view)
{
  if ((! opts.labels.on) && (! opts.lines.on) && (! opts.frame.on))
    return;

  if (! isReady ())
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
  std::vector<glGrib::String::align_t> align;

  createStr (opts.E, glGrib::String::E, view, S, X, Y, A, align);
  createStr (opts.W, glGrib::String::W, view, S, X, Y, A, align);
  createStr (opts.N, glGrib::String::N, view, S, X, Y, A, align);
  createStr (opts.S, glGrib::String::S, view, S, X, Y, A, align);

  if (opts.labels.on)
    {
      clear (labels);
      labels.setShared (false);
      labels.setChange (false);

      glGrib::FontPtr font = getGlGribFontPtr (opts.labels.font); 

      labels.setup (font, S, X, Y, opts.labels.font.scale, align, A);
      labels.setForegroundColor (opts.labels.font.color.foreground);
      labels.setBackgroundColor (opts.labels.font.color.background);
    }


  if (opts.lines.on)
    {
      ticks.VAID.clear ();

      // Coordinates of ticks
      std::vector<glm::vec3> XYa (S.size ());

      for (size_t i = 0; i < XYa.size (); i++)
        {
          XYa[i].x = X[i];
          XYa[i].y = Y[i];
          XYa[i].z = align[i];
        }

      vertexbuffer = glGrib::OpenGLBufferPtr<glm::vec3> (XYa);

      numberOfTicks = XYa.size ();
    }

}

template <>
void glGrib::Ticks::ticks_t::setupVertexAttributes () const
{
  glGrib::Program * program = glGrib::Program::load ("TICKS");
  ticks->vertexbuffer->bind (GL_ARRAY_BUFFER);
  auto attr = program->getAttributeLocation ("xya");
  glEnableVertexAttribArray (attr); 
  glVertexAttribPointer (attr, 3, GL_FLOAT, GL_FALSE, 0, nullptr); 
  glVertexAttribDivisor (attr, 1);
}

template <>
void glGrib::Ticks::frame_t::setupVertexAttributes () const
{
  // Needed to use a shader
}


