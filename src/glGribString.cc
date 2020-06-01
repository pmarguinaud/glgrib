#include "glGribString.h"
#include "glGribTrigonometry.h"
#include "glGribClear.h"

#include <iostream>
#include <stdexcept>

glGrib::String & glGrib::String::operator= (const glGrib::String & str)
{
  if (this != &str)
    {
      clear (this->d);
      this->VAID.clear ();
      if (str.d.ready)
        {
          if (str.d.shared)
            {
              d = str.d;   
              VAID = str.VAID;
              d.ready = true;
	    }
	  else if (str.d.change)
            {
              setup (str.d.font, str.d.data, str.d.x, str.d.y, str.d.scale, 
                     str.d.align, str.d.X, str.d.Y, str.d.Z, str.d.A);
              d.color0 = str.d.color0;
              d.color1 = str.d.color1;
	      d.shared = str.d.shared;
	      d.change = str.d.change;
	    }
	  else
	    {
              throw std::runtime_error (std::string ("Cannot copy string"));
	    }
        }
    }
  return *this;
}

void glGrib::String::setup3D (glGrib::const_FontPtr ff, const std::vector<std::string> & str, 
	                    const std::vector<float> & _X, const std::vector<float> & _Y,
	                    const std::vector<float> & _Z, const std::vector<float> & _A,
	                    float s, align_t _align)
{
  std::vector<float> _x, _y;
  for (size_t i = 0; i < str.size (); i++)
    {
      _x.push_back (0.0f);
      _y.push_back (0.0f);
    }
  setup (ff, str, _x, _y, s, std::vector<align_t>{_align}, _X, _Y, _Z, _A);
}

void glGrib::String::setup2D (glGrib::const_FontPtr ff, const std::vector<std::string> & str, 
                              float x, float y, float s, align_t align)
{
  setup (ff, str, std::vector<float>{x}, std::vector<float>{y}, s, std::vector<align_t>{align});
}

void glGrib::String::setup2D (glGrib::const_FontPtr ff, const std::vector<std::string> & str, 
                              const std::vector<float> & x, const std::vector<float> & y, 
                              float s, align_t align,
                              const std::vector<float> & a)
{
  setup (ff, str, x, y, s, std::vector<align_t>{align},
         std::vector<float>{}, std::vector<float>{}, std::vector<float>{}, a);
}

void glGrib::String::setup2D (glGrib::const_FontPtr ff, const std::vector<std::string> & str, 
                              const std::vector<float> & x, const std::vector<float> & y, 
                              float s, const std::vector<align_t> & align,
                              const std::vector<float> & a)
{
  setup (ff, str, x, y, s, align, 
         std::vector<float>{}, std::vector<float>{}, std::vector<float>{}, a);
}

void glGrib::String::setup2D (glGrib::const_FontPtr ff, const std::string & str, 
                            float x, float y, float s, align_t align)
{
  std::vector<std::string> _str = {str};
  std::vector<float>       _x   = {x};
  std::vector<float>       _y   = {y};
  setup (ff, _str, _x, _y, s, std::vector<align_t>{align});
}

void glGrib::String::setup (glGrib::const_FontPtr ff, const std::vector<std::string> & str, 
                            const std::vector<float> & _x, const std::vector<float> & _y, 
                            float s, const std::vector<align_t> & _align,
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
  for (size_t i = 0; i < d.data.size (); i++)
    d.len += d.data[i].size ();

  std::vector<float> xy, let, xyz;
  xy.reserve (4 * d.len);
  let.reserve (d.len);
  xyz.reserve (4 * d.len);
  
  d.font = ff;
  d.scale = s;

  float dx = d.scale * d.font->getAspect ();
  float dy = d.scale;
     
  float posb = dy * d.font->getPosBelow ();
  float posu = dy * d.font->getPosAbove ();
  float dym  = dy - posu - posb;

  float x0 = 0.0f, y0 = 0.0f; // last coordinates used

  // For each string
  for (size_t j = 0, ii = 0; j < d.data.size (); j++)
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
     
      align_t _align;

      if (j < d.align.size ())
        _align = d.align[j];
      else
        _align = d.align[0];

      if (_align & CX)
        xx = xx - (len * dx / 2);
      else if (_align & EX)
        xx = xx - len * dx;
     
      if (_align & CY)
        yy = yy - dym / 2;
      else if (_align & NY)
        yy = yy - dym;
     
      float X = 0.0f, Y = 0.0f, Z = 0.0f, A = 0.0f;
      
      // 2D setup : average position
      if (j < _X.size ())
        X = _X[j];
      else
        X = xx + dx * len / 2;

      if (j < _Y.size ())
        Y = _Y[j];
      else
        Y = yy + dym / 2;

      if (j < _Z.size ())
        Z = _Z[j];

      A = j < _A.size () ?  _A[j] : 0.0f;

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
     

  d.vertexbuffer = glGrib::OpenGLBufferPtr<float> (xy);
  d.letterbuffer = glGrib::OpenGLBufferPtr<float> (let);
  d.xyzbuffer = glGrib::OpenGLBufferPtr<float> (xyz);

  d.ready = true;
  
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

void glGrib::String::setupVertexAttributes () const
{
  glGrib::Program * program = d.font->getProgram ();

  auto pattr = program->getAttributeLocation ("letterPos");
  auto vattr = program->getAttributeLocation ("letterVal");
  auto xattr = program->getAttributeLocation ("letterXYZA");

  d.vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (pattr); 
  glVertexAttribPointer (pattr, 4, GL_FLOAT, GL_FALSE, 0, nullptr); 
  glVertexAttribDivisor (pattr, 1);
  
  d.letterbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (vattr); 
  glVertexAttribPointer (vattr, 1, GL_FLOAT, GL_FALSE, 0, nullptr); 
  glVertexAttribDivisor (vattr, 1);
  
  d.xyzbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (xattr); 
  glVertexAttribPointer (xattr, 4, GL_FLOAT, GL_FALSE, 0, nullptr); 
  glVertexAttribDivisor (xattr, 1);
}


void glGrib::String::render (const glGrib::View & view) const
{
  if (! d.ready)
    return;

  d.font->select ();

  glGrib::Program * program = d.font->getProgram ();

  view.setMVP (program);

  float length = view.pixelToDistAtNadir (10);

  program->set ("scale", d.scale);
  program->set ("texture", 0);
  program->set ("l3d", 1);
  program->set ("ratio", view.getRatio ());
  program->set ("color0", d.color0);
  program->set ("color1", d.color1);
  program->set ("length10", length);
  program->set ("scaleXYZ", d.scaleXYZ);

  VAID.bind ();
  unsigned int ind[6] = {0, 1, 2, 2, 3, 0};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, d.len);
  VAID.unbind ();

  view.delMVP (program);
}

void glGrib::String::render (const glm::mat4 & MVP) const
{
  if (! d.ready)
    return;

  d.font->select ();

  glGrib::Program * program = d.font->getProgram ();
  program->set ("MVP", MVP);
  program->set ("scale", d.scale);
  program->set ("scaleXYZ", 1.0f);
  program->set ("texture", 0);
  program->set ("l3d", 0);
  program->set ("color0", d.color0);
  program->set ("color1", d.color1);

  VAID.bind ();
  unsigned int ind[12] = {0, 1, 2, 2, 3, 0};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, d.len);
  VAID.unbind ();
}

void glGrib::String::update (const std::string & str)
{
  update (std::vector<std::string>{str});
}

void glGrib::String::update (const std::vector<std::string> & str)
{
  if (! d.ready)
    throw std::runtime_error (std::string ("Cannot set update string"));
  if (! d.change)
    throw std::runtime_error (std::string ("Cannot set update string"));

  if (str.size () > d.data.size ())
    return;
  for (size_t i = 0; i < str.size (); i++)
    if (str[i].size () > d.data[i].size ())
      return;

  for (size_t i = 0; i < str.size (); i++)
    {
      for (size_t j = 0; j < str[i].size (); j++)
        d.data[i][j] = str[i][j];
      for (size_t j = str[i].size (); j < d.data[i].size (); j++)
        d.data[i][j] = ' ';
    }
  for (size_t i = str.size (); i < d.data.size (); i++)
    for (size_t j = 0; j < d.data[i].size (); j++)
      d.data[i][j] = ' ';

  auto let = d.letterbuffer->map ();

  for (size_t j = 0, ii = 0; j < d.data.size (); j++)
    for (size_t i = 0; i < d.data[j].size (); i++, ii++) 
      {
        int rank = d.font->map (d.data[j][i]);
        let[ii] = rank; 
      }
}


void glGrib::String2D::render (const glm::mat4 & MVP) const
{
  if (! isReady ())
    return;

  str.d.font->select ();

  glGrib::Program * program = str.d.font->getProgram ();
  program->set ("MVP", MVP);
  program->set ("scale", str.d.scale);
  program->set ("scaleXYZ", 1.0f);
  program->set ("texture", 0);
  program->set ("l3d", 0);
  program->set ("color0", str.d.color0);
  program->set ("color1", str.d.color1);

  str.VAID.bind ();
  unsigned int ind[12] = {0, 1, 2, 2, 3, 0};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, str.d.len);
  str.VAID.unbind ();
}

void glGrib::String2D::setup (glGrib::const_FontPtr ff, const std::vector<std::string> & str, 
                              float x, float y, float s, String::align_t align)
{
  this->str.setup (ff, str, std::vector<float>{x}, std::vector<float>{y}, s, std::vector<String::align_t>{align});
  setReady ();
}

void glGrib::String2D::setup (glGrib::const_FontPtr ff, const std::vector<std::string> & str, 
                              const std::vector<float> & x, const std::vector<float> & y, 
                              float s, String::align_t align,
                              const std::vector<float> & a)
{
  this->str.setup (ff, str, x, y, s, std::vector<String::align_t>{align},
         std::vector<float>{}, std::vector<float>{}, std::vector<float>{}, a);
  setReady ();
}

void glGrib::String2D::setup (glGrib::const_FontPtr ff, const std::vector<std::string> & str, 
                              const std::vector<float> & x, const std::vector<float> & y, 
                              float s, const std::vector<String::align_t> & align,
                              const std::vector<float> & a)
{
  this->str.setup (ff, str, x, y, s, align, 
         std::vector<float>{}, std::vector<float>{}, std::vector<float>{}, a);
  setReady ();
}

void glGrib::String2D::setup (glGrib::const_FontPtr ff, const std::string & str, 
                              float x, float y, float s, String::align_t align)
{
  std::vector<std::string> _str = {str};
  std::vector<float>       _x   = {x};
  std::vector<float>       _y   = {y};
  this->str.setup (ff, _str, _x, _y, s, std::vector<String::align_t>{align});
  setReady ();
}

void glGrib::String3D::setup (glGrib::const_FontPtr ff, const std::vector<std::string> & str, 
	                      const std::vector<float> & _X, const std::vector<float> & _Y,
	                      const std::vector<float> & _Z, const std::vector<float> & _A,
	                      float s, String::align_t _align)
{
  std::vector<float> _x, _y;
  for (size_t i = 0; i < str.size (); i++)
    {
      _x.push_back (0.0f);
      _y.push_back (0.0f);
    }
  this->str.setup (ff, str, _x, _y, s, std::vector<String::align_t>{_align}, _X, _Y, _Z, _A);
  setReady ();
}


void glGrib::String3D::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  if (! isReady ())
    return;

  str.d.font->select ();

  glGrib::Program * program = str.d.font->getProgram ();

  view.setMVP (program);

  float length = view.pixelToDistAtNadir (10);

  program->set ("scale", str.d.scale);
  program->set ("texture", 0);
  program->set ("l3d", 1);
  program->set ("ratio", view.getRatio ());
  program->set ("color0", str.d.color0);
  program->set ("color1", str.d.color1);
  program->set ("length10", length);
  program->set ("scaleXYZ", str.d.scaleXYZ);

  str.VAID.bind ();
  unsigned int ind[6] = {0, 1, 2, 2, 3, 0};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, str.d.len);
  str.VAID.unbind ();

  view.delMVP (program);
}


