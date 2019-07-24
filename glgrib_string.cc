#include "glgrib_string.h"

#include <iostream>

glgrib_string & glgrib_string::operator= (const glgrib_string & str)
{
  if (this != &str)
    {
      cleanup ();
      if (str.ready)
        {
          for (int i =0; i < 3; i++)
            color0[i] = str.color0[i];
          init (str.font, str.data, str.x, str.y, str.scale, str.align, str.X, str.Y, str.Z);
        }
    }
}



void glgrib_string::cleanup ()
{
  if (ready)
    {
      glDeleteBuffers (1, &xyzbuffer);
      glDeleteBuffers (1, &vertexbuffer);
      glDeleteBuffers (1, &letterbuffer);
      glDeleteVertexArrays (1, &VertexArrayID);
    }
  ready = false;
}

glgrib_string::~glgrib_string ()
{
  cleanup ();
}

void glgrib_string::init (const_glgrib_font_ptr ff, const std::vector<std::string> & str, 
                          float x, float y, float s, align_t align)
{
  init (ff, str, std::vector<float>{x}, std::vector<float>{y}, s, align);
}

void glgrib_string::init (const_glgrib_font_ptr ff, const std::vector<std::string> & str, 
                          const std::vector<float> & _x, const std::vector<float> & _y, 
                          float s, align_t _align,
			  const std::vector<float> & _X, 
			  const std::vector<float> & _Y,
			  const std::vector<float> & _Z)
{
  data = str;
  x = _x;
  y = _y;
  X = _X;
  Y = _Y;
  Z = _Z;
  align = _align;
  
  len = 0; // Total number of letters
  for (int i = 0; i < data.size (); i++)
    len += data[i].size ();

  std::vector<float> xy, let, xyz;
  xy.reserve (4 * len);
  let.reserve (len);
  xyz.reserve (3 * len);
  
  color0[0] = 1.;
  color0[1] = 1.;
  color0[2] = 1.;

  font = ff;
  scale = s;
     

  float dx = scale * font->getAspect ();
  float dy = scale;
     
  float posb = dy * font->getPosBelow ();
  float posu = dy * font->getPosAbove ();
  float dym  = dy - posu - posb;

  float x0, y0; // last coordinates used

  // For each string
  for (int j = 0, ii = 0; j < data.size (); j++)
    {
      int len = data[j].size ();
     
      float xx, yy; // Base position

      // Use coordinates if provided or compute position
      if (j < x.size ())
        xx = x[j];
      else
        xx = x0;

      // Use coordinates if provided or compute position
      if (j < y.size ())
        yy = y[j];
      else
        yy = y0 - dy;

      x0 = xx;
      y0 = yy;

      yy = yy - posb;
     
      if (align & CX)
        xx = xx - (len * dx / 2);
      else if (align & EX)
        xx = xx - len * dx;
     
      if (align & CY)
        yy = yy - dym / 2;
      else if (align & NY)
        yy = yy - dym;
     
      float X = j < _X.size () ?  _X[j] : 0.0f;
      float Y = j < _Y.size () ?  _Y[j] : 0.0f;
      float Z = j < _Z.size () ?  _Z[j] : 0.0f;

      for (int i = 0; i < len; i++, ii++)
        {
          int rank = font->map (data[j][i]);
     
	  xy.push_back (xx);
	  xy.push_back (yy);
	  xy.push_back (dx);
	  xy.push_back (dy);
          let.push_back (rank);
          xyz.push_back (X); 
          xyz.push_back (Y); 
          xyz.push_back (Z); 
     
	  // Advance
          xx = xx + dx;
        }

    }
     
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  glGenBuffers (1, &vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, xy.size () * sizeof (float), xy.data (), GL_STATIC_DRAW);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, NULL); 
  glVertexAttribDivisor (0, 1);
  
  glGenBuffers (1, &letterbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, letterbuffer);
  glBufferData (GL_ARRAY_BUFFER, let.size () * sizeof (float), let.data (), GL_STATIC_DRAW);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, 1, GL_FLOAT, GL_FALSE, 0, NULL); 
  glVertexAttribDivisor (1, 1);
  
  glGenBuffers (1, &xyzbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, xyzbuffer);
  glBufferData (GL_ARRAY_BUFFER, xyz.size () * sizeof (float), xyz.data (), GL_STATIC_DRAW);
  glEnableVertexAttribArray (2); 
  glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  glVertexAttribDivisor (2, 1);
  
  ready = true;

}


void glgrib_string::init (const_glgrib_font_ptr ff, const std::string & str, 
                          float x, float y, float s, align_t align)
{
  std::vector<std::string> _str = {str};
  std::vector<float>       _x   = {x};
  std::vector<float>       _y   = {y};
  init (ff, _str, _x, _y, s, align);
}

void glgrib_string::render (const glgrib_view & view) const
{
  if (! ready)
    return;

  font->select ();

  glgrib_program * program = font->getProgram ();
  view.setMVP (program);
  program->set1f ("scale", scale);
  program->set1i ("texture", 0);
  program->set1i ("l3d", 1);
  program->set3fv ("color0", color0);
  

  glBindVertexArray (VertexArrayID);
  unsigned int ind[12] = {0, 1, 2, 2, 3, 0};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, len);
}

void glgrib_string::render (const glm::mat4 & MVP) const
{
  if (! ready)
    return;

  font->select ();

  glgrib_program * program = font->getProgram ();
  program->setMatrix4fv ("MVP", &MVP[0][0]);
  program->set1f ("scale", scale);
  program->set1i ("texture", 0);
  program->set1i ("l3d", 0);
  program->set3fv ("color0", color0);
  
  glBindVertexArray (VertexArrayID);
  unsigned int ind[12] = {0, 1, 2, 2, 3, 0};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, len);
}

void glgrib_string::update (const std::string & str)
{
  update (std::vector<std::string>{str});
}

void glgrib_string::update (const std::vector<std::string> & str)
{
  if (str.size () > data.size ())
    return;
  for (int i = 0; i < str.size (); i++)
    if (str[i].size () > data[i].size ())
      return;

  for (int i = 0; i < str.size (); i++)
    {
      for (int j = 0; j < str[i].size (); j++)
        data[i][j] = str[i][j];
      for (int j = str[i].size (); j < data[i].size (); j++)
        data[i][j] = ' ';
    }
  for (int i = str.size (); i < data.size (); i++)
    for (int j = 0; j < data[i].size (); j++)
      data[i][j] = ' ';

  glBindBuffer (GL_ARRAY_BUFFER, letterbuffer);
  float * let = (float *)glMapBufferRange (GL_ARRAY_BUFFER, 0, len * sizeof (float), 
  	                                   GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

  for (int j = 0, ii = 0; j < data.size (); j++)
    for (int i = 0; i < data[j].size (); i++, ii++) 
      {
        int rank = font->map (data[j][i]);
        let[ii] = rank; 
      }

  glFlushMappedBufferRange (GL_ARRAY_BUFFER, 0, len * sizeof (float));
  glUnmapBuffer (GL_ARRAY_BUFFER);
}



