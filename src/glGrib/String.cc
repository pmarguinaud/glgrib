#include "glGrib/String.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/Clear.h"

#include <iostream>
#include <stdexcept>

namespace glGrib
{

StringTypes::align_t StringTypes::str2align 
  (const std::string & str)
{
#define S2A(x) do { if (str == #x) return x; } while (0)
    S2A (C);  S2A (N);  S2A (S); 
    S2A (W);  S2A (E);  S2A (NE); 
    S2A (SE); S2A (NW); S2A (SW);
#undef S2A
  return C;
}

template <bool SHARED, bool CHANGE>
template <bool OK>
typename std::enable_if<OK,String<SHARED,CHANGE> &>::type
String<SHARED,CHANGE>::operator=
   (const String<SHARED,CHANGE> & str)
{
  if (this != &str)
    {
      clear (this->d);
      this->VAID.clear ();
      if (str.d.ready)
        do_copy (str);
    }
  return *this;
}

template <>
void String<1,0>::do_copy (const String & str)
{
  d = str.d;   
  VAID = str.VAID;
}

template <>
void String<1,1>::do_copy (const String & str)
{
  d = str.d;   
  VAID = str.VAID;
}

template <>
void String<0,1>::do_copy (const String & str)
{
  setup (str.d.font, str.d.data, str.d.x, str.d.y, str.d.scale, 
         str.d.align, str.d.X, str.d.Y, str.d.Z, str.d.A);
  d.color0 = str.d.color0;
  d.color1 = str.d.color1;
  d.shared = str.d.shared;
  d.change = str.d.change;
}

template <bool SHARED, bool CHANGE>
void String<SHARED,CHANGE>::setup
  (const_FontPtr ff, const string_v & str, 
   const float_v & _x, const float_v & _y, 
   float s, const align_v & _align,
   const float_v & _X, const float_v & _Y,
   const float_v & _Z, const float_v & _A)
{
  using namespace StringTypes;
  d.data = str;
  d.x = _x;
  d.y = _y;
  d.X = _X; d.Y = _Y;
  d.Z = _Z; d.A = _A;
  d.align = _align;
  
  d.len = 0; // Total number of letters
  for (size_t i = 0; i < d.data.size (); i++)
    d.len += d.data[i].size ();

  float_v xy, let, xyz;
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
     

  d.vertexbuffer = OpenGLBufferPtr<float> (xy);
  d.letterbuffer = OpenGLBufferPtr<float> (let);
  d.xyzbuffer = OpenGLBufferPtr<float> (xyz);

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

template <bool SHARED, bool CHANGE>
void String<SHARED,CHANGE>::setupVertexAttributes () const
{
  Program * program = d.font->getProgram ();

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

template <bool SHARED, bool CHANGE>
template <bool OK>
typename std::enable_if<OK,void>::type
String<SHARED,CHANGE>::do_update (const string_v & str)
{
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

template void String<1,1>::do_update (const string_v &);
template void String<0,1>::do_update (const string_v &);


template <bool SHARED, bool CHANGE>
void String2D<SHARED,CHANGE>::render (const glm::mat4 & MVP) const
{
  if (! isReady ())
    return;

  str.d.font->select ();

  Program * program = str.d.font->getProgram ();
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

template <bool SHARED, bool CHANGE>
void String3D<SHARED,CHANGE>::render
  (const View & view, const OptionsLight & light) 
const
{
  if (! isReady ())
    return;

  str.d.font->select ();

  Program * program = str.d.font->getProgram ();

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


#define DEF(SHARED,CHANGE) \
template class String<SHARED,CHANGE>; \
template class String2D<SHARED,CHANGE>; \
template class String3D<SHARED,CHANGE>

DEF (1, 0);
DEF (0, 1);
DEF (1, 1);
DEF (0, 0);

}
