#include "glgrib_mapscale.h"
#include "glgrib_shader.h"


glgrib_mapscale & glgrib_mapscale::operator= (const glgrib_mapscale & mapscale)
{
  if (this != &mapscale)
    {
      cleanup ();
      if (mapscale.ready)
        init (mapscale.opts);
    }
}

void glgrib_mapscale::init (const glgrib_options_mapscale & o)
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

  glgrib_font_ptr font = new_glgrib_font_ptr (opts.font);

  label.init2D (font, std::string ("coucou"), opts.position.xmax, opts.position.ymax, opts.font.scale, glgrib_string::S);
  label.setForegroundColor (opts.font.color.foreground.r / 255.0f, 
                            opts.font.color.foreground.g / 255.0f, 
                            opts.font.color.foreground.b / 255.0f);

  ready = true;
}

void glgrib_mapscale::cleanup ()
{
  if (ready)
    {
      glDeleteBuffers (1, &elementbuffer);
      glDeleteVertexArrays (1, &VertexArrayID);
    }
  ready = false;
}

glgrib_mapscale::~glgrib_mapscale ()
{
  cleanup ();
}

void glgrib_mapscale::render (const glm::mat4 & MVP) const
{
  if (! ready)
    return;

  label.render (MVP);

  program.use ();

// label.update (str);

  program.setMatrix4fv ("MVP", &MVP[0][0]);
  program.set1f ("xmin", opts.position.xmin);
  program.set1f ("xmax", opts.position.xmax);
  program.set1f ("ymin", opts.position.ymin);
  program.set1f ("ymax", opts.position.ymax);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, NULL);
//glDrawElements (GL_TRIANGLES, 3 * 3, GL_UNSIGNED_INT, NULL);


}

glgrib_program glgrib_mapscale::program = glgrib_program
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
  
  if (false){
  rank = gl_VertexID / 4;
  int corn = int (mod (gl_VertexID, 4));
  int ix = int (mod (corn, 2));
  int iy = corn / 2;
  x = xmin + xmax * ix;
  y = ymin + iy * (ymax - ymin) / 4.0;
  }else{
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

  x = xmin + ix * (xmax - xmin) / 2.0;
  y = ymin + iy * (ymax - ymin) / 1.0;
  }

  vec2 vertexPos = vec2 (x, y);
  gl_Position =  MVP * vec4 (0., vertexPos.x, vertexPos.y, 1.);
}


)CODE");


