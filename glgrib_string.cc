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
          init (str.font, str.data, str.x, str.y, str.scale, str.align);
        }
    }
}



void glgrib_string::cleanup ()
{
  if (ready)
    {
      glDeleteBuffers (1, &vertexbuffer);
      glDeleteBuffers (1, &letterbuffer);
      glDeleteBuffers (1, &elementbuffer);
      glDeleteVertexArrays (1, &VertexArrayID);
    }
  ready = false;
}

glgrib_string::~glgrib_string ()
{
  cleanup ();
}

void glgrib_string::init (const_glgrib_font_ptr ff, const std::vector<std::string> & str, 
                          float x, float y, float s, glgrib_string_align_t align)
{
  init (ff, str, std::vector<float>{x}, std::vector<float>{y}, s, align);
}

void glgrib_string::init (const_glgrib_font_ptr ff, const std::vector<std::string> & str, 
                          const std::vector<float> & _x, const std::vector<float> & _y, 
                          float s, glgrib_string_align_t _align)
{
  data = str;
  x = _x;
  y = _y;
  align = _align;
  
  int len = 0;
  for (int i = 0; i < data.size (); i++)
    len += data[i].size ();

  int np = 4 * len;
  nt = 2 * len;

  float * xy = new float[2*np];
  float * let = new float[3*np];
  unsigned int *ind = new unsigned int[3*nt];
  
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

  for (int j = 0, ii = 0; j < data.size (); j++)
    {
      int len = data[j].size ();
     
      float xx, yy;

      if (j < x.size ())
        xx = x[j];
      else
        xx = x0;

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
     
      for (int i = 0; i < len; i++, ii++)
        {
          int rank = font->map (data[j][i]);
     
          xy[8*ii+2*0+0] = xx   ; xy[8*ii+2*0+1] = yy   ;
          xy[8*ii+2*1+0] = xx+dx; xy[8*ii+2*1+1] = yy   ; 
          xy[8*ii+2*2+0] = xx+dx; xy[8*ii+2*2+1] = yy+dy; 
          xy[8*ii+2*3+0] = xx   ; xy[8*ii+2*3+1] = yy+dy; 
     
          let[12*ii+3*0+0] = xx; let[12*ii+3*0+1] = yy; let[12*ii+3*0+2] = rank; 
          let[12*ii+3*1+0] = xx; let[12*ii+3*1+1] = yy; let[12*ii+3*1+2] = rank; 
          let[12*ii+3*2+0] = xx; let[12*ii+3*2+1] = yy; let[12*ii+3*2+2] = rank; 
          let[12*ii+3*3+0] = xx; let[12*ii+3*3+1] = yy; let[12*ii+3*3+2] = rank; 
     
          ind[6*ii+0] = 4*ii+0; ind[6*ii+1] = 4*ii+1; ind[6*ii+2] = 4*ii+2; 
          ind[6*ii+3] = 4*ii+2; ind[6*ii+4] = 4*ii+3; ind[6*ii+5] = 4*ii+0; 
     
          xx = xx + dx;
        }

    }
     
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  glGenBuffers (1, &vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 2 * np * sizeof (float), xy, GL_STATIC_DRAW);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  glGenBuffers (1, &letterbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, letterbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * np * sizeof (float), let, GL_STATIC_DRAW);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * nt * sizeof (unsigned int), ind , GL_STATIC_DRAW);

  ready = true;

  delete [] xy;
  delete [] let;
  delete [] ind;

}


void glgrib_string::init (const_glgrib_font_ptr ff, const std::string & str, 
                          float x, float y, float s, glgrib_string_align_t align)
{
  std::vector<std::string> _str = {str};
  std::vector<float>       _x   = {x};
  std::vector<float>       _y   = {y};
  init (ff, _str, _x, _y, s, align);
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
  program->set3fv ("color0", color0);
  
  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, NULL);
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

  int len = 0;
  for (int i = 0; i < data.size (); i++)
    len += data[i].size ();

  int np = 4 * len;

  glBindBuffer (GL_ARRAY_BUFFER, letterbuffer);
  float * let = (float *)glMapBufferRange (GL_ARRAY_BUFFER, 0, 3 * np * sizeof (float), 
  	                                   GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

  for (int j = 0, ii = 0; j < data.size (); j++)
    for (int i = 0; i < data[j].size (); i++, ii++) 
      {
        int rank = font->map (data[j][i]);
        let[12*ii+3*0+2] = rank; 
        let[12*ii+3*1+2] = rank; 
        let[12*ii+3*2+2] = rank; 
        let[12*ii+3*3+2] = rank; 
      }

  glFlushMappedBufferRange (GL_ARRAY_BUFFER, 0, 3 * np * sizeof (float));
  glUnmapBuffer (GL_ARRAY_BUFFER);
}



