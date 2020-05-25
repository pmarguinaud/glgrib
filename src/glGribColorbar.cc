#include "glGribColorbar.h"
#include "glGribShader.h"


glGrib::Colorbar & glGrib::Colorbar::operator= (const glGrib::Colorbar & colorbar)
{
  if (this != &colorbar)
    {
      clear ();
      if (colorbar.ready)
        setup (colorbar.opts);
    }
  return *this;
}

void glGrib::Colorbar::setup (const glGrib::OptionsColorbar & o)
{
  opts = o;

  nt = 2 * 256;

  unsigned int * ind = new unsigned int[3*nt];

  for (int i = 0, ii = 0, jj = 0; i < 256; i++)
    {
      ind[3*ii+0] = jj+2; ind[3*ii+1] = jj+0; ind[3*ii+2] = jj+1; ii++;
      ind[3*ii+0] = jj+3; ind[3*ii+1] = jj+2; ind[3*ii+2] = jj+1; ii++;
      jj += 4;
    }


  elementbuffer = newGlgribOpenGLBufferPtr (3 * nt * sizeof (unsigned int), ind);

  delete [] ind;

  ready = true;
}

void glGrib::Colorbar::setupVertexAttributes () const
{
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
}

void glGrib::Colorbar::clear ()
{
  if (ready)
    {
      VAID.clear ();
      label.clear ();
    }
  palette = glGrib::Palette ();
  ready = false;
}

void glGrib::Colorbar::createLabels (std::vector<float> & x, std::vector<float> & y, 
                                     std::vector<std::string> & str,
                                     const std::vector<float> & values, const bool nonlinear)
{
  for (size_t i = 0; i < values.size (); i++)
    {
      float val = values[i];
      char tmp[32];
      sprintf (tmp, opts.format.c_str (), palette.getOffset () + val * palette.getScale ());
      std::string s = std::string (tmp);
      while (s.length () < 6)
        s += " ";
      str.push_back (s);
      x.push_back (opts.position.xmin-0.01f);

      float yv;

      if (nonlinear)
        yv = (opts.position.ymax - opts.position.ymin) * (val - palette.getMin ()) 
                    / (palette.getMax () - palette.getMin ()) + opts.position.ymin;
      else
        yv = (opts.position.ymax - opts.position.ymin) * static_cast<float> (i)
                    / (values.size () - 1) + opts.position.ymin;

      y.push_back (yv);

    }
}

void glGrib::Colorbar::updateNonLinear (const float min, const float max, 
                                        std::vector<float> & x, std::vector <float> & y,
                                        std::vector<std::string> & str) 
{
  const std::vector<float> & values_pal = palette.getValues ();
  std::vector<float> values;

  if (opts.levels.values.size () > 0)
    {
      // Take values passed as options in [min..max]
      for (size_t i = 0; i < opts.levels.values.size (); i++)
        if ((min <= opts.levels.values[i]) && (opts.levels.values[i] <= max))
          values.push_back (opts.levels.values[i]);
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
      float d = (max - min) / (opts.levels.number - 1);
      for (int i = 0; i < opts.levels.number; i++)
        values.push_back (min + d * i);
    }
  
  createLabels (x, y, str, values, true);

  size_t ncolors = palette.size ();
  for (int i = 0; i < 256; i++)
    rank2rgba[i] = 1 + (i * (ncolors-1)) / 256;
}

void glGrib::Colorbar::updateLinear (const float min, const float max, 
                                     std::vector<float> & x, std::vector <float> & y,
                                     std::vector<std::string> & str) 
{
  const std::vector<float> & values_pal = palette.getValues ();

  createLabels (x, y, str, values_pal, false);

  rank2rgba[0] = 0;
  for (size_t i = 0; i < values_pal.size () - 1; i++)
    {
      int j1 = 1 + (255 * (i + 0)) / (values_pal.size () - 1);
      int j2 = 1 + (255 * (i + 1)) / (values_pal.size () - 1);
      int k = palette.getColorIndex (values_pal[i+1]);
      for (int j = j1; j < j2; j++)
        rank2rgba[j] = k;
    }
}

void glGrib::Colorbar::update (const glGrib::Palette & p) 
{

  if (! ready)
    return;

  if (p == palette)
    return;

  palette = p;

  rank2rgba.resize (256);

  label.clear ();
  
  const float min = palette.getMin (), max = palette.getMax ();
  
  glGrib::FontPtr font = newGlgribFontPtr (opts.font);
  
  std::vector<std::string> str;
  std::vector<float> x, y;
  
  if (! palette.isLinear ())
    updateNonLinear (min, max, x, y, str);
  else
    updateLinear (min, max, x, y, str);
  
  label.setup2D (font, str, x, y, opts.font.scale, glGrib::String::SE);
  label.setForegroundColor (opts.font.color.foreground);
  label.setBackgroundColor (opts.font.color.background);
  
  label.update (str);

}

void glGrib::Colorbar::render (const glm::mat4 & MVP) const
{
  if (! ready)
    return;

  label.render (MVP);

  glGrib::Program * program = glGrib::Program::load ("COLORBAR");

  program->use ();

  palette.set (program);

  program->set ("MVP", MVP);
  program->set ("rank2rgba", rank2rgba);
  program->set ("xmin", opts.position.xmin);
  program->set ("xmax", opts.position.xmax);
  program->set ("ymin", opts.position.ymin);
  program->set ("ymax", opts.position.ymax);

  VAID.bind ();
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, nullptr);
  VAID.unbind ();


}

