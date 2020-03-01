#include "glGribColorbar.h"
#include "glGribShader.h"


glGribColorbar & glGribColorbar::operator= (const glGribColorbar & colorbar)
{
  if (this != &colorbar)
    {
      clear ();
      if (colorbar.ready)
        setup (colorbar.opts);
    }
  return *this;
}

void glGribColorbar::setup (const glGribOptionsColorbar & o)
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

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * nt * sizeof (unsigned int), ind , GL_STATIC_DRAW);
  
  delete [] ind;

  program.compile ();

  ready = true;
}

void glGribColorbar::clear ()
{
  if (ready)
    {
      glDeleteBuffers (1, &elementbuffer);
      glDeleteVertexArrays (1, &VertexArrayID);
      label.clear ();
    }
  pref = glGribPalette ();
  ready = false;
}

glGribColorbar::~glGribColorbar ()
{
  clear ();
}

void glGribColorbar::render (const glm::mat4 & MVP, const glGribPalette & p,
                              float valmin, float valmax) const
{
  if (! ready)
    return;

  rank2rgba.resize (256);

  glGribPalette p1 = p;
  if (! p1.hasMin ())
    p1.setMin (valmin);
  if (! p1.hasMax ())
    p1.setMax (valmax);


  if (p1 != pref)
    {
      label.clear ();

      pref = p1;

      float min = pref.getMin (), max = pref.getMax ();

      glgrib_font_ptr font = newGlgribFontPtr (opts.font);
     
      std::vector<std::string> str;
      std::vector<float> x, y;

      std::vector<float> values;

      const std::vector<float> & values_pal = pref.getValues ();

      if (! pref.isLinear ())
        {
          if (opts.levels.values.size () > 0)
            {
              for (int i = 0; i < opts.levels.values.size (); i++)
                if ((min <= opts.levels.values[i]) && (opts.levels.values[i] <= max))
                  values.push_back (opts.levels.values[i]);
            }
          else if (values_pal.size () > 0)
            {
              for (int i = 0; i < values_pal.size (); i++)
                if ((min <= values_pal[i]) && (values_pal[i] <= max))
                  values.push_back (values_pal[i]);
            }
          else
            {
              float d = (max - min) / (opts.levels.number - 1);
              for (int i = 0; i < opts.levels.number; i++)
                values.push_back (min + d * i);
            }
          
          for (int i = 0; i < values.size (); i++)
            {
              float val = values[i];
              char tmp[32];
              sprintf (tmp, opts.format.c_str (), pref.getOffset () + val * pref.getScale ());
              std::string s = std::string (tmp);
              while (s.length () < 6)
                s += " ";
              str.push_back (s);
              x.push_back (opts.position.xmin-0.01f);
              y.push_back ((opts.position.ymax - opts.position.ymin) * (val - pref.getMin ()) 
                            / (pref.getMax () - pref.getMin ()) + opts.position.ymin);
            }

          for (int i = 0; i < 256; i++)
            rank2rgba[i] = i;
        }
      else
        {
          for (int i = 0; i < values_pal.size (); i++)
            {
              float val = values_pal[i];
              char tmp[32];
              sprintf (tmp, opts.format.c_str (), pref.getOffset () + val * pref.getScale ());
              std::string s = std::string (tmp);
              while (s.length () < 6)
                s += " ";
              str.push_back (s);
              x.push_back (opts.position.xmin-0.01f);
              y.push_back ((opts.position.ymax - opts.position.ymin) * i 
                            / (values_pal.size () - 1) + opts.position.ymin);
            }

	  rank2rgba[0] = 0;
          for (int i = 0; i < values_pal.size () - 1; i++)
            {
              int j1 = 1 + (255 * (i + 0)) / (values_pal.size () - 1);
              int j2 = 1 + (255 * (i + 1)) / (values_pal.size () - 1);
              int k = pref.getColorIndex (values_pal[i+1]);
              for (int j = j1; j < j2; j++)
                rank2rgba[j] = k;
	    }
        }

      label.setup2D (font, str, x, y, opts.font.scale, glGribString::SE);
      label.setForegroundColor (opts.font.color.foreground);
      label.setBackgroundColor (opts.font.color.background);

      label.update (str);
    }

  label.render (MVP);

  program.use ();

  pref.setRGBA255 (program.programID);

  program.set ("MVP", MVP);
  program.set ("rank2rgba", rank2rgba);

  program.set ("xmin", opts.position.xmin);
  program.set ("xmax", opts.position.xmax);
  program.set ("ymin", opts.position.ymin);
  program.set ("ymax", opts.position.ymax);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, nullptr);


}

glGribProgram glGribColorbar::program = glGribProgram
(
R"CODE(
#version 330 core

flat in int rank;

out vec4 color;

uniform vec4 RGBA0[256];
uniform int rank2rgba[256];

void main ()
{
  if(false){
  color.r = float (rank) / 255.;
  color.g = float (rank) / 255.;
  color.b = float (rank) / 255.;
  color.a = float (rank) / 255.;
  }else{
  color = RGBA0[rank2rgba[rank]];
  }
  if(false)
  if(rank2rgba[rank] == 255)
    color = vec4 (1., 1., 1., 1.);
}
)CODE",
R"CODE(

#version 330 core

uniform mat4 MVP;
uniform float xmin = 0.08;
uniform float xmax = 0.10; 
uniform float ymin = 0.05; 
uniform float ymax = 0.95;

flat out int rank;

void main()
{
  rank = gl_VertexID / 4;
  int corn = int (mod (gl_VertexID, 4));
  int ix = int (mod (corn, 2));
  int iy = corn / 2;
  float x = xmin + (xmax - xmin) * ix;
  float y = ymin + (rank + iy) * (ymax - ymin) / 255.0;
  vec2 vertexPos = vec2 (x, y);
  gl_Position =  MVP * vec4 (0., vertexPos.x, vertexPos.y, 1.);
}


)CODE");


