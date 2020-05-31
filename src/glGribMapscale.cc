#include "glGribMapscale.h"
#include "glGribShader.h"


void glGrib::Mapscale::setup (const glGrib::OptionsMapscale & o)
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

  elementbuffer = glGrib::OpenGLBufferPtr<unsigned int> (3 * nt, ind);
  
  delete [] ind;

  glGrib::FontPtr font = newGlgribFontPtr (opts.font);

  label.setup2D (font, std::string (15, ' '), opts.position.xmin, opts.position.ymax + 0.01, opts.font.scale, glGrib::String::SW);
  label.setForegroundColor (opts.font.color.foreground);
  label.setBackgroundColor (opts.font.color.background);

  ready = true;
}

void glGrib::Mapscale::setupVertexAttributes () const
{
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
}

void glGrib::Mapscale::clear ()
{
  if (ready)
    VAID.clear ();
  ready = false;
}

void glGrib::Mapscale::render (const glm::mat4 & MVP, const glGrib::View & view) const
{
  if (! ready)
    return;

  const double a = 6371229.0;

  const double frac0 = 0.25;
  float dist0 = a * view.fracToDistAtNadir (frac0);

  double frac1 = opts.position.xmax - opts.position.xmin;
  float dist1 = dist0 * frac1 / frac0;

  int ndigits = 0;
  float pow10 = 1;
  while (static_cast<int>(dist1 / pow10))
    {
      ndigits++;
      pow10 = pow10 * 10;
    }
  
  ndigits--;
  pow10 = pow10 / 10;

  dist1 = pow10 * static_cast<int>(dist1 / pow10);
  frac1 = frac0 * dist1 / dist0;

  std::string str;
  if (dist1 > 1000)
    str = std::to_string (static_cast<int>(dist1/1000)) + " km";
  else
    str = std::to_string (static_cast<int>(dist1)) + " m";

  str = str.substr (0, 15);
  
  if (label_str != str)
    {
      label.update (str);
      label_str = str;
    }

  label.render (MVP);

  glGrib::Program * program = glGrib::Program::load ("MAPSCALE");

  program->use ();

  program->set ("MVP", MVP);
  program->set ("xmin", opts.position.xmin);
  program->set ("xmax", opts.position.xmin + frac1);
  program->set ("ymin", opts.position.ymin);
  program->set ("ymax", opts.position.ymax);
  
  VAID.bind ();
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, nullptr);
  VAID.unbind ();

}

