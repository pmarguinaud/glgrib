#ifndef _GLGRIB_PROJECTION_H
#define _GLGRIB_PROJECTION_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class glgrib_projection
{
public:
  enum
  {
    XYZ=0,
    POLAR_NORTH=1,
    POLAR_SOUTH=2,
    MERCATOR=3,
    LATLON=4 
  };
  virtual glm::vec3 project (const glm::vec3 &) const = 0;
  virtual int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const = 0;
  virtual glm::mat4 getView (const glm::vec3 &, const float) const = 0;
  virtual int getType () const = 0;
  virtual bool setLon0 (const float &) const = 0;
};

class glgrib_projection_xyz : public glgrib_projection
{
public:
  virtual glm::vec3 project (const glm::vec3 &) const;
  virtual int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const;
  virtual glm::mat4 getView (const glm::vec3 &, const float) const;
  virtual int getType () const { return glgrib_projection::XYZ; }
  virtual bool setLon0 (const float &) const { return false; }
};

class glgrib_projection_latlon : public glgrib_projection
{
public:
  virtual glm::vec3 project (const glm::vec3 &) const;
  virtual int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const;
  virtual glm::mat4 getView (const glm::vec3 &, const float) const;
  virtual int getType () const { return glgrib_projection::LATLON; }
  virtual bool setLon0 (const float & lon) const { lon0 = lon; return true; }
private:
  mutable float lon0 = 180.0; // Latitude of right handside
};

class glgrib_projection_mercator : public glgrib_projection
{
public:
  virtual glm::vec3 project (const glm::vec3 &) const;
  virtual int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const;
  virtual glm::mat4 getView (const glm::vec3 &, const float) const;
  virtual int getType () const { return glgrib_projection::MERCATOR; }
  virtual bool setLon0 (const float & lon) const { lon0 = lon; return true; }
private:
  mutable float lon0 = 180.0; // Latitude of right handside
};

class glgrib_projection_polar_north : public glgrib_projection
{
public:
  virtual glm::vec3 project (const glm::vec3 &) const;
  virtual int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const;
  virtual glm::mat4 getView (const glm::vec3 &, const float) const;
  virtual int getType () const { return glgrib_projection::POLAR_NORTH; }
  virtual bool setLon0 (const float &) const { return false; }
};

class glgrib_projection_polar_south : public glgrib_projection
{
public:
  virtual glm::vec3 project (const glm::vec3 &) const;
  virtual int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const;
  virtual glm::mat4 getView (const glm::vec3 &, const float) const;
  virtual int getType () const { return glgrib_projection::POLAR_SOUTH; }
  virtual bool setLon0 (const float &) const { return false; }
};

class glgrib_projection_set 
{
public:
  glgrib_projection_set () 
  {
    init ();
  }
  glgrib_projection_set & operator= (const glgrib_projection_set & ps)
  {
    init ();
  }
  glgrib_projection * current () const
  {
    return proj[current_];
  }
  glgrib_projection * next ()
  {
    current_++; current_ %= 5; return current ();
  }
private:
  int current_ = 4;
  void init ()
  { 
     proj[0] = &proj_xyz;
     proj[1] = &proj_polar_north;
     proj[2] = &proj_polar_south;
     proj[3] = &proj_mercator;
     proj[4] = &proj_latlon;
  }
  glgrib_projection * proj[5];
  glgrib_projection_xyz         proj_xyz;
  glgrib_projection_polar_north proj_polar_north;
  glgrib_projection_polar_south proj_polar_south;
  glgrib_projection_mercator    proj_mercator;
  glgrib_projection_latlon      proj_latlon;
};


#endif
