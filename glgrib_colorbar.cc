#include "glgrib_colorbar.h"
#include "glgrib_shader.h"


glgrib_colorbar & glgrib_colorbar::operator= (const glgrib_colorbar & colorbar)
{
  if (this != &colorbar)
    {
      cleanup ();
      if (colorbar.ready)
        init (colorbar.opts);
    }
}

void glgrib_colorbar::init (const glgrib_options_colorbar & o)
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

  glgrib_font_ptr font = new_glgrib_font_ptr (opts.font);

  std::vector<std::string> str;
  std::vector<float> x, y;

  char tmp[256];
  sprintf (tmp, opts.format.c_str (), 0.0);

  for (int i = 0; i < opts.levels; i++)
    {
      str.push_back (std::string (tmp));
      x.push_back (0.01f);
      y.push_back ((opts.position.ymax - opts.position.ymin) * i / 10.0f + opts.position.ymin);
    }
  label.init2D (font, str, x, y, opts.font.scale, glgrib_string::SW);
  label.setForegroundColor (opts.font.color.foreground.r / 255.0f, 
                            opts.font.color.foreground.g / 255.0f, 
                            opts.font.color.foreground.b / 255.0f);

  ready = true;
}

void glgrib_colorbar::cleanup ()
{
  if (ready)
    {
      glDeleteBuffers (1, &elementbuffer);
      glDeleteVertexArrays (1, &VertexArrayID);
    }
  pref = glgrib_palette ();
  ready = false;
}

glgrib_colorbar::~glgrib_colorbar ()
{
  cleanup ();
}

void glgrib_colorbar::render (const glm::mat4 & MVP, const glgrib_palette & p,
                              float valmin, float valmax) const
{
  if (! ready)
    return;

  label.render (MVP);

  program.use ();

  glgrib_palette p1 = p;
  if (! p1.hasMin ())
    p1.min = valmin;
  if (! p1.hasMax ())
    p1.max = valmax;

  if (p1 != pref)
    {
      pref = p1;
      std::vector<std::string> str;
      for (int i = 0; i < opts.levels; i++)
        {
          char tmp[32];
          sprintf (tmp, opts.format.c_str (), pref.min + i * (pref.max - pref.min) / 10.0f);
	  std::string s (tmp);
	  while (s.length () < 6)
	    s += " ";
          str.push_back (s);
        }

      label.update (str);
      pref.setRGBA255 (program.programID);
    }

  program.setMatrix4fv ("MVP", &MVP[0][0]);
  program.set1f ("xmin", opts.position.xmin);
  program.set1f ("xmax", opts.position.xmax);
  program.set1f ("ymin", opts.position.ymin);
  program.set1f ("ymax", opts.position.ymax);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, NULL);


}

glgrib_program glgrib_colorbar::program = glgrib_program
(
R"CODE(
#version 330 core

flat in int rank;

out vec4 color;

uniform vec4 RGBA0[256];

void main ()
{
  if(false){
  color.r = float (rank) / 255.;
  color.g = float (rank) / 255.;
  color.b = float (rank) / 255.;
  color.a = float (rank) / 255.;
  }else{
  color = RGBA0[rank];
  }
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
  float x = xmin + xmax * ix;
  float y = ymin + (rank + iy) * (ymax - ymin) / 255.0;
  vec2 vertexPos = vec2 (x, y);
  gl_Position =  MVP * vec4 (0., vertexPos.x, vertexPos.y, 1.);
}


)CODE");


