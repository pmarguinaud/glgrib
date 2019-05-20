#include "glgrib_colorbar.h"
#include "glgrib_shader.h"


GLuint glgrib_colorbar::programID = 0;
bool glgrib_colorbar::programReady = false;


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

  loadShader ();

  glgrib_font_ptr font = new_glgrib_font_ptr (opts.font);

  std::vector<std::string> str;
  std::vector<float> x, y;
  for (int i = 0; i < 11; i++)
    {
      str.push_back ("XXXXXX");
      x.push_back (0.01f);
      y.push_back ((ymax - ymin) * i / 10.0f + ymin);
    }
  label.init (font, str, x, y, opts.font.scale, glgrib_string::SW);
  label.setColor (1., 1., 1.);

  ready = true;
}

void glgrib_colorbar::cleanup ()
{
  if (ready)
    {
      glDeleteBuffers (1, &elementbuffer);
      glDeleteVertexArrays (1, &VertexArrayID);
    }
  ready = false;
}

glgrib_colorbar::~glgrib_colorbar ()
{
  cleanup ();
}

void glgrib_colorbar::render (const glm::mat4 & MVP, const glgrib_palette & p) const
{
  if (! ready)
    return;

  if (p != pref)
    {
      pref = p;

      std::vector<std::string> str;
      for (int i = 0; i < 11; i++)
        {
          char tmp[32];
          sprintf (tmp, "%6.4g", p.min + i * (p.max - p.min) / 10.0f);
	  std::string s (tmp);
	  while (s.length () < 6)
	    s += " ";
          str.push_back (s);
          std::cout << s << std::endl;
        }


      label.update (str);
    }


  label.render (MVP);

  glUseProgram (programID);

  pref.setRGBA255 (programID);

  glUniformMatrix4fv (glGetUniformLocation (programID, "MVP"), 1, GL_FALSE, &MVP[0][0]);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, NULL);


}

void glgrib_colorbar::loadShader ()
{
  if (programReady)
    return;

  programID = glgrib_load_shader
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

  programReady = true;
}

