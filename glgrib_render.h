#ifndef _GLGRIB_RENDER_H
#define _GLGRIB_RENDER_H

typedef struct view_t
{
  float rc = 6.0, latc = 0., lonc = 0., fov = 20.;
  GLuint MatrixID;
} view_t;

typedef struct prog_t
{
  GLuint programID;
} prog_t;

typedef struct obj_t
{
  GLuint VertexArrayID;
  GLuint vertexbuffer, colorbuffer, elementbuffer;
  int use_alpha = 1;
  unsigned int ncol, nt;
  int np;
} obj_t;

extern view_t View;

void display (const prog_t * prog, const obj_t * world, const obj_t * cube, const view_t * view);

void world_init (obj_t * obj, const char * file);

void cube_init (obj_t * obj);

void view_init (prog_t * prog, view_t * view);

void view_free (view_t * view);
      
void obj_free (obj_t * obj);

void prog_free (prog_t * prog);

void prog_init (prog_t * prog);

void gl_init ();
  
#endif
