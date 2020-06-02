#include "glGribColorbar.h"
#include "glGribShader.h"
#include "glGribClear.h"


glGrib::Colorbar & glGrib::Colorbar::operator= (const glGrib::Colorbar & colorbar)
{
  if (this != &colorbar)
    {
      clear (d);
      glGrib::Object2D::operator= (colorbar);
      VAID.clear ();
      if (colorbar.isReady ())
        setup (colorbar.d.opts);
    }
  return *this;
}

void glGrib::Colorbar::setup (const glGrib::OptionsColorbar & o)
{
  if (! o.on)
    return;

  d.opts = o;

  d.nt = 2 * 256;

  unsigned int * ind = new unsigned int[3*d.nt];

  for (int i = 0, ii = 0, jj = 0; i < 256; i++)
    {
      ind[3*ii+0] = jj+2; ind[3*ii+1] = jj+0; ind[3*ii+2] = jj+1; ii++;
      ind[3*ii+0] = jj+3; ind[3*ii+1] = jj+2; ind[3*ii+2] = jj+1; ii++;
      jj += 4;
    }


  d.elementbuffer = glGrib::OpenGLBufferPtr<unsigned int> (3 * d.nt, ind);

  delete [] ind;

  setReady ();
}

void glGrib::Colorbar::setupVertexAttributes () const
{
  d.elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
}

void glGrib::Colorbar::createLabels (std::vector<float> & x, std::vector<float> & y, 
                                     std::vector<std::string> & str,
                                     const std::vector<float> & values, const bool nonlinear)
{
  for (size_t i = 0; i < values.size (); i++)
    {
      float val = values[i];
      char tmp[32];
      sprintf (tmp, d.opts.format.c_str (), d.palette.getOffset () + val * d.palette.getScale ());
      std::string s = std::string (tmp);
      while (s.length () < 6)
        s += " ";
      str.push_back (s);
      x.push_back (d.opts.position.xmin-0.01f);

      float yv;

      if (nonlinear)
        yv = (d.opts.position.ymax - d.opts.position.ymin) * (val - d.palette.getMin ()) 
                    / (d.palette.getMax () - d.palette.getMin ()) + d.opts.position.ymin;
      else
        yv = (d.opts.position.ymax - d.opts.position.ymin) * static_cast<float> (i)
                    / (values.size () - 1) + d.opts.position.ymin;

      y.push_back (yv);

    }
}

void glGrib::Colorbar::updateNonLinear (const float min, const float max, 
                                        std::vector<float> & x, std::vector <float> & y,
                                        std::vector<std::string> & str) 
{
  const std::vector<float> & values_pal = d.palette.getValues ();
  std::vector<float> values;

  if (d.opts.levels.values.size () > 0)
    {
      // Take values passed as options in [min..max]
      for (size_t i = 0; i < d.opts.levels.values.size (); i++)
        if ((min <= d.opts.levels.values[i]) && (d.opts.levels.values[i] <= max))
          values.push_back (d.opts.levels.values[i]);
    }
  else if (values_pal.size () > 0)
    {
      // Take values from palette
      for (size_t i = 0; i < values_pal.size (); i++)
        if ((min <= values_pal[i]) && (values_pal[i] <= max))
          values.push_back (values_pal[i]);
    }
  else
    {
      // Create a range
      float e = (max - min) / (d.opts.levels.number - 1);
      for (int i = 0; i < d.opts.levels.number; i++)
        values.push_back (min + e * i);
    }
  
  createLabels (x, y, str, values, true);

  size_t ncolors = d.palette.size ();
  for (int i = 0; i < 256; i++)
    d.rank2rgba[i] = 1 + (i * (ncolors-1)) / 256;
}

void glGrib::Colorbar::updateLinear (const float min, const float max, 
                                     std::vector<float> & x, std::vector <float> & y,
                                     std::vector<std::string> & str) 
{
  const std::vector<float> & values_pal = d.palette.getValues ();

  createLabels (x, y, str, values_pal, false);

  d.rank2rgba[0] = 0;
  for (size_t i = 0; i < values_pal.size () - 1; i++)
    {
      int j1 = 1 + (255 * (i + 0)) / (values_pal.size () - 1);
      int j2 = 1 + (255 * (i + 1)) / (values_pal.size () - 1);
      int k = d.palette.getColorIndex (values_pal[i+1]);
      for (int j = j1; j < j2; j++)
        d.rank2rgba[j] = k;
    }
}

void glGrib::Colorbar::update (const glGrib::Palette & p) 
{

  if (! isReady ())
    return;

  if (p == d.palette)
    return;

  d.palette = p;

  d.rank2rgba.resize (256);

  clear (d.label);
  
  const float min = d.palette.getMin (), max = d.palette.getMax ();
  
  glGrib::FontPtr font = getGlGribFontPtr (d.opts.font);
  
  std::vector<std::string> str;
  std::vector<float> x, y;
  
  if (! d.palette.isLinear ())
    updateNonLinear (min, max, x, y, str);
  else
    updateLinear (min, max, x, y, str);
  
  d.label.setup (font, str, x, y, d.opts.font.scale, glGrib::String::SE);
  d.label.setForegroundColor (d.opts.font.color.foreground);
  d.label.setBackgroundColor (d.opts.font.color.background);
  
  d.label.update (str);

}

void glGrib::Colorbar::render (const glm::mat4 & MVP) const
{
  if (! isReady ())
    return;

  d.label.render (MVP);

  glGrib::Program * program = glGrib::Program::load ("COLORBAR");

  program->use ();

  d.palette.set (program);

  program->set ("MVP", MVP);
  program->set ("rank2rgba", d.rank2rgba);
  program->set ("xmin", d.opts.position.xmin);
  program->set ("xmax", d.opts.position.xmax);
  program->set ("ymin", d.opts.position.ymin);
  program->set ("ymax", d.opts.position.ymax);

  VAID.bind ();
  glDrawElements (GL_TRIANGLES, 3 * d.nt, GL_UNSIGNED_INT, nullptr);
  VAID.unbind ();


}

