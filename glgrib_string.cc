#include "glgrib_string.h"

#include <iostream>
#include <stdexcept>

glgrib_string & glgrib_string::operator= (const glgrib_string & str)
{
  if (this != &str)
    {
      clear ();
      if (str.ready)
        {
          if (d.shared)
            {
              d = str.d;
	      setupVertexAttributes ();
	    }
	  else if (d.change)
            {
              setup (str.d.font, str.d.data, str.d.x, str.d.y, str.d.scale, 
                    str.d.align, str.d.X, str.d.Y, str.d.Z, str.d.A);
              for (int i =0; i < 4; i++)
                d.color0[i] = str.d.color0[i];
              for (int i =0; i < 4; i++)
                d.color1[i] = str.d.color1[i];
	      d.shared = str.d.shared;
	      d.change = str.d.change;
	    }
	  else
	    {
              throw std::runtime_error (std::string ("Cannot copy string"));
	    }
        }
    }
}



void glgrib_string::clear ()
{
  if (ready)
    glDeleteVertexArrays (1, &VertexArrayID);
  ready = false;
}

glgrib_string::~glgrib_string ()
{
  clear ();
}

void glgrib_string::setup3D (const_glgrib_font_ptr ff, const std::vector<std::string> & str, 
	                    const std::vector<float> & _X, const std::vector<float> & _Y,
	                    const std::vector<float> & _Z, const std::vector<float> & _A,
	                    float s, align_t _align)
{
  std::vector<float> _x, _y;
  for (int i = 0; i < str.size (); i++)
    {
      _x.push_back (0.0f);
      _y.push_back (0.0f);
    }
  setup (ff, str, _x, _y, s, _align, _X, _Y, _Z, _A);
}

void glgrib_string::setup2D (const_glgrib_font_ptr ff, const std::vector<std::string> & str, 
                            float x, float y, float s, align_t align)
{
  setup (ff, str, std::vector<float>{x}, std::vector<float>{y}, s, align);
}

void glgrib_string::setup2D (const_glgrib_font_ptr ff, const std::vector<std::string> & str, 
                            const std::vector<float> & x, const std::vector<float> & y, float s, align_t align)
{
  setup (ff, str, x, y, s, align);
}

void glgrib_string::setup (const_glgrib_font_ptr ff, const std::vector<std::string> & str, 
                          const std::vector<float> & _x, const std::vector<float> & _y, 
                          float s, align_t _align,
			  const std::vector<float> & _X, const std::vector<float> & _Y,
			  const std::vector<float> & _Z, const std::vector<float> & _A)
{
  d.data = str;
  d.x = _x;
  d.y = _y;
  d.X = _X; d.Y = _Y;
  d.Z = _Z; d.A = _A;
  d.align = _align;
  
  d.len = 0; // Total number of letters
  for (int i = 0; i < d.data.size (); i++)
    d.len += d.data[i].size ();

  std::vector<float> xy, let, xyz;
  xy.reserve (4 * d.len);
  let.reserve (d.len);
  xyz.reserve (3 * d.len);
  
  for (int i = 0; i < 4; i++)
    d.color0[i] = 1.;
  for (int i = 0; i < 4; i++)
    d.color1[i] = 0.;

  d.font = ff;
  d.scale = s;
     

  float dx = d.scale * d.font->getAspect ();
  float dy = d.scale;
     
  float posb = dy * d.font->getPosBelow ();
  float posu = dy * d.font->getPosAbove ();
  float dym  = dy - posu - posb;

  float x0, y0; // last coordinates used

  // For each string
  for (int j = 0, ii = 0; j < d.data.size (); j++)
    {
      int len = d.data[j].size ();
     
      float xx, yy; // Base position

      // Use coordinates if provided or compute position
      if (j < d.x.size ())
        xx = d.x[j];
      else
        xx = x0;

      // Use coordinates if provided or compute position
      if (j < d.y.size ())
        yy = d.y[j];
      else
        yy = y0 - dy;

      x0 = xx;
      y0 = yy;

      yy = yy - posb;
     
      if (d.align & CX)
        xx = xx - (len * dx / 2);
      else if (d.align & EX)
        xx = xx - len * dx;
     
      if (d.align & CY)
        yy = yy - dym / 2;
      else if (d.align & NY)
        yy = yy - dym;
     
      const float deg2rad = M_PI / 180.0;
      float X = j < _X.size () ?  _X[j] : 0.0f;
      float Y = j < _Y.size () ?  _Y[j] : 0.0f;
      float Z = j < _Z.size () ?  _Z[j] : 0.0f;
      float A = j < _A.size () ?  _A[j] : 0.0f;

      for (int i = 0; i < len; i++, ii++)
        {
          int rank = d.font->map (d.data[j][i]);
     
	  xy.push_back (xx);
	  xy.push_back (yy);
	  xy.push_back (dx);
	  xy.push_back (dy);
          let.push_back (rank);
          xyz.push_back (X); 
          xyz.push_back (Y); 
          xyz.push_back (Z); 
          xyz.push_back (A * deg2rad); 
     
	  // Advance
          xx = xx + dx;
        }

    }
     

  d.vertexbuffer = new_glgrib_opengl_buffer_ptr (xy.size () * sizeof (float), xy.data ());
  d.letterbuffer = new_glgrib_opengl_buffer_ptr (let.size () * sizeof (float), let.data ());
  d.xyzbuffer = new_glgrib_opengl_buffer_ptr (xyz.size () * sizeof (float), xyz.data ());

  setupVertexAttributes ();
  
  if (! d.change)
    {
      d.x.clear ();
      d.y.clear ();
      d.X.clear ();
      d.Y.clear ();
      d.Z.clear ();
      d.A.clear ();
    }
}

void glgrib_string::setupVertexAttributes ()
{
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  d.vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, NULL); 
  glVertexAttribDivisor (0, 1);
  
  d.letterbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, 1, GL_FLOAT, GL_FALSE, 0, NULL); 
  glVertexAttribDivisor (1, 1);
  
  d.xyzbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (2); 
  glVertexAttribPointer (2, 4, GL_FLOAT, GL_FALSE, 0, NULL); 
  glVertexAttribDivisor (2, 1);
  glBindVertexArray (0);
  

  ready = true;
}


void glgrib_string::setup2D (const_glgrib_font_ptr ff, const std::string & str, 
                            float x, float y, float s, align_t align)
{
  std::vector<std::string> _str = {str};
  std::vector<float>       _x   = {x};
  std::vector<float>       _y   = {y};
  setup (ff, _str, _x, _y, s, align);
}

void glgrib_string::render (const glgrib_view & view) const
{
  if (! ready)
    return;

  d.font->select ();

  glgrib_program * program = d.font->getProgram ();
  view.setMVP (program);

  float length = view.pixel_to_dist_at_nadir (10);

  program->set1f ("scale", d.scale);
  program->set1i ("texture", 0);
  program->set1i ("l3d", 2);
  program->set4fv ("color0", d.color0);
  program->set4fv ("color1", d.color1);
  program->set1f ("length10", length);


  glBindVertexArray (VertexArrayID);
  unsigned int ind[6] = {0, 1, 2, 2, 3, 0};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, d.len);
}

void glgrib_string::render (const glm::mat4 & MVP) const
{
  if (! ready)
    return;

  d.font->select ();

  glgrib_program * program = d.font->getProgram ();
  program->setMatrix4fv ("MVP", &MVP[0][0]);
  program->set1f ("scale", d.scale);
  program->set1i ("texture", 0);
  program->set1i ("l3d", 0);
  program->set4fv ("color0", d.color0);
  program->set4fv ("color1", d.color1);

  glBindVertexArray (VertexArrayID);
  unsigned int ind[12] = {0, 1, 2, 2, 3, 0};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, d.len);
}

void glgrib_string::update (const std::string & str)
{
  update (std::vector<std::string>{str});
}

void glgrib_string::update (const std::vector<std::string> & str)
{
  if (! ready)
    throw std::runtime_error (std::string ("Cannot set update string"));
  if (! d.change)
    throw std::runtime_error (std::string ("Cannot set update string"));

  if (str.size () > d.data.size ())
    return;
  for (int i = 0; i < str.size (); i++)
    if (str[i].size () > d.data[i].size ())
      return;

  for (int i = 0; i < str.size (); i++)
    {
      for (int j = 0; j < str[i].size (); j++)
        d.data[i][j] = str[i][j];
      for (int j = str[i].size (); j < d.data[i].size (); j++)
        d.data[i][j] = ' ';
    }
  for (int i = str.size (); i < d.data.size (); i++)
    for (int j = 0; j < d.data[i].size (); j++)
      d.data[i][j] = ' ';

  d.letterbuffer->bind (GL_ARRAY_BUFFER);

  float * let = (float *)glMapBufferRange (GL_ARRAY_BUFFER, 0, d.len * sizeof (float), 
  	                                   GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

  for (int j = 0, ii = 0; j < d.data.size (); j++)
    for (int i = 0; i < d.data[j].size (); i++, ii++) 
      {
        int rank = d.font->map (d.data[j][i]);
        let[ii] = rank; 
      }

  glFlushMappedBufferRange (GL_ARRAY_BUFFER, 0, d.len * sizeof (float));
  glUnmapBuffer (GL_ARRAY_BUFFER);
}

void glgrib_string::setShared (bool p)
{
  d.shared = p;
}

void glgrib_string::setChange (bool u)
{
  if (ready && u)
    throw std::runtime_error (std::string ("Cannot set attribute change"));
  d.change = u;
}



