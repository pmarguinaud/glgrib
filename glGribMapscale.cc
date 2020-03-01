#include "glGribMapscale.h"
#include "glGribShader.h"


glGribMapscale & glGribMapscale::operator= (const glGribMapscale & mapscale)
{
  if (this != &mapscale)
    {
      clear ();
      if (mapscale.ready)
        setup (mapscale.opts);
    }
  return *this;
}

void glGribMapscale::setup (const glGribOptionsMapscale & o)
{
  opts = o;

  nt = 2 * 4;

  unsigned int * ind = new unsigned int[3*nt];

  for (int i = 0, ii = 0, jj = 0; i < 4; i++)
    {
      ind[3*ii+0] = jj+2; ind[3*ii+1] = jj+0; ind[3*ii+2] = jj+1; ii++;
      ind[3*ii+0] = jj+3; ind[3*ii+1] = jj+2; ind[3*ii+2] = jj+1; ii++;
      jj += 4;
    }

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * nt * sizeof (unsigned int), ind , GL_STATIC_DRAW);
  
  delete [] ind;

  program.compile ();

  glgrib_font_ptr font = newGlgribFontPtr (opts.font);

  label.setup2D (font, std::string (15, ' '), opts.position.xmin, opts.position.ymax + 0.01, opts.font.scale, glGribString::SW);
  label.setForegroundColor (opts.font.color.foreground);
  label.setBackgroundColor (opts.font.color.background);

  ready = true;
}

void glGribMapscale::clear ()
{
  if (ready)
    {
      glDeleteBuffers (1, &elementbuffer);
      glDeleteVertexArrays (1, &VertexArrayID);
    }
  ready = false;
}

glGribMapscale::~glGribMapscale ()
{
  clear ();
}

void glGribMapscale::render (const glm::mat4 & MVP, const glGribView & view) const
{
  if (! ready)
    return;

  const double a = 6371229.0;

  const double frac0 = 0.25;
  float dist0 = a * view.frac_to_dist_at_nadir (frac0);

  double frac1 = opts.position.xmax - opts.position.xmin;
  float dist1 = dist0 * frac1 / frac0;

  int ndigits = 0;
  float pow10 = 1;
  while ((int)(dist1 / pow10))
    {
      ndigits++;
      pow10 = pow10 * 10;
    }
  
  ndigits--;
  pow10 = pow10 / 10;

  dist1 = pow10 * (int)(dist1 / pow10);
  frac1 = frac0 * dist1 / dist0;

  std::string str;
  if (dist1 > 1000)
    str = std::to_string ((int)(dist1/1000)) + " km";
  else
    str = std::to_string ((int)dist1) + " m";

  str = str.substr (0, 15);
  
  if (label_str != str)
    {
      label.update (str);
      label_str = str;
    }

  label.render (MVP);
  program.use ();

  program.set ("MVP", MVP);
  program.set ("xmin", opts.position.xmin);
  program.set ("xmax", opts.position.xmin + frac1);
  program.set ("ymin", opts.position.ymin);
  program.set ("ymax", opts.position.ymax);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, nullptr);

}

glGribProgram glGribMapscale::program = glGribProgram
(
R"CODE(
#version 330 core

out vec4 color;

flat in int rank;

uniform vec4 color1 = vec4 (1.0, 1.0, 1.0, 1.0);
uniform vec4 color2 = vec4 (0.3, 0.3, 0.3, 1.0);

void main ()
{
  if (rank == 0)
    {
      color.r = color1.r;
      color.g = color1.g;
      color.b = color1.b;
      color.a = color1.a;
    }
  else
    {
      color.r = color2.r;
      color.g = color2.g;
      color.b = color2.b;
      color.a = color2.a;
    }
}
)CODE",
R"CODE(

#version 330 core

uniform mat4 MVP;
uniform float xmin = 0.10;
uniform float xmax = 0.20; 
uniform float ymin = 0.10; 
uniform float ymax = 0.12;

flat out int rank;

void main()
{
  float x, y;
  
  rank = int (mod (gl_VertexID / 4, 2));

  int ix = int (gl_VertexID / 4);

  int k = int (mod (gl_VertexID, 4));
  
  int iy = int (mod (gl_VertexID, 2));

  if (iy == 0)
    {
      if (k == 0)
        ix = ix + 1;
      else if (k == 3)
        ix = ix + 1;
    }
  else
    {
      if (k == 1)
        ix = ix + 1;
      else if (k == 2)
        ix = ix + 1;
    }

  x = xmin + ix * (xmax - xmin) / 4.0;
  y = ymin + iy * (ymax - ymin) / 1.0;

  vec2 vertexPos = vec2 (x, y);
  gl_Position =  MVP * vec4 (0., vertexPos.x, vertexPos.y, 1.);
}


)CODE");


