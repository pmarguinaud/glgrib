#include "glGrib/Mapscale.h"
#include "glGrib/Shader.h"
#include "glGrib/Buffer.h"


namespace glGrib
{

void Mapscale::setup (const OptionsMapscale & o)
{
  if (! o.on)
    return;

  opts = o;

  nt = 2 * 4;

  Buffer<unsigned int> ind (3 * nt);

  for (int i = 0, ii = 0, jj = 0; i < 4; i++)
    {
      ind[3*ii+0] = jj+2; ind[3*ii+1] = jj+0; ind[3*ii+2] = jj+1; ii++;
      ind[3*ii+0] = jj+3; ind[3*ii+1] = jj+2; ind[3*ii+2] = jj+1; ii++;
      jj += 4;
    }

  elementbuffer = OpenGLBufferPtr<unsigned int> (ind);
  
  FontPtr font = getGlGribFontPtr (opts.font);

  label.setup (font, std::string (15, ' '), opts.position.xmin, opts.position.ymax + 0.01, opts.font.bitmap.scale, StringTypes::SW);
  label.setForegroundColor (opts.font.color.foreground);
  label.setBackgroundColor (opts.font.color.background);

  setReady ();
}

void Mapscale::setupVertexAttributes () const
{
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
}

void Mapscale::reSize (const View & view) 
{
  if (! isReady ())
    return;

  const double a = 6371229.0;

  const double frac0 = 0.25;
  float dist0 = a * view.fracToDistAtNadir (frac0);

  frac1 = opts.position.xmax - opts.position.xmin;

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

}

void Mapscale::render (const glm::mat4 & MVP) const
{
  if (! isReady ())
    return;

  label.render (MVP);

  Program * program = Program::load ("MAPSCALE");

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

}
