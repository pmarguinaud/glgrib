#ifndef _GLGRIB_LAND_H
#define _GLGRIB_LAND_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"
#include "glgrib_geometry.h"
#include "glgrib_options.h"

class glgrib_land : public glgrib_object
{
public:
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual ~glgrib_land () { clear (); }
  const_glgrib_geometry_ptr getGeometry () const { return geometry; }
  virtual void clear ();
  virtual void setup (const glgrib_options_land &);
  virtual void resize (const glgrib_view &) {}
  void triangulate (std::vector<int> *, std::vector<int> *,
                    std::vector<int> *, std::vector<int> *,
                    const std::vector<unsigned int> &,
                    std::vector<float> *, std::vector<int> *,
                    std::vector<unsigned int> *);
  void subdivide (const std::vector<int> &, const std::vector<int> &,
                  const std::vector<int> &, const std::vector<int> &,
                  std::vector<unsigned int> *, std::vector<float> *);
protected:
  glgrib_options_land opts;
  glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
  const_glgrib_geometry_ptr geometry;
  int numberOfPoints;
  unsigned int numberOfTriangles;
  GLuint VertexArrayID = 0;
};

#endif
