#ifndef _GLGRIB_RENDER_H
#define _GLGRIB_RENDER_H

#include <list>

class prog_t
{
public:
  GLuint programID;
  void init ();
  ~prog_t ();
};

class view_t
{
public:
  float rc = 6.0, latc = 0., lonc = 0., fov = 20.;
  GLuint MatrixID;
  void init (prog_t *);
};

class obj_t
{
  public:
    virtual void render () const = 0;
};

class polyhedron_t : public obj_t
{
public:
  virtual void render () const;
  ~polyhedron_t ();
  GLuint VertexArrayID;
  GLuint vertexbuffer, colorbuffer, elementbuffer;
  int use_alpha = 1;
  unsigned int ncol, nt;
  int np;
};

class world_t : public polyhedron_t
{
public:
  void init (const char *);
};

class cube_t : public polyhedron_t
{
public:
  void init ();
};

class scene_t
{
public:
  void display () const;
  std::list<obj_t*> objlist;
  view_t * view;
  prog_t * prog;
};

void gl_init ();
  
#endif
