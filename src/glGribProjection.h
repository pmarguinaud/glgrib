#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace glGrib
{

class Projection
{
public:
  enum type
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
  static type typeFromString (std::string str);
};

class ProjectionXYZ : public Projection
{
public:
  glm::vec3 project (const glm::vec3 &) const override;
  int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const override;
  glm::mat4 getView (const glm::vec3 &, const float) const override;
  int getType () const override { return Projection::XYZ; }
  bool setLon0 (const float &) const override { return false; }
};

class ProjectionLatLon : public Projection
{
public:
  glm::vec3 project (const glm::vec3 &) const override;
  int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const override;
  glm::mat4 getView (const glm::vec3 &, const float) const override;
  int getType () const override { return Projection::LATLON; }
  bool setLon0 (const float & lon) const override { lon0 = lon; return true; }
private:
  mutable float lon0 = 180.0; // Latitude of right handside
};

class ProjectionMercator : public Projection
{
public:
  glm::vec3 project (const glm::vec3 &) const override;
  int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const override;
  glm::mat4 getView (const glm::vec3 &, const float) const override;
  int getType () const override { return Projection::MERCATOR; }
  bool setLon0 (const float & lon) const override { lon0 = lon; return true; }
private:
  mutable float lon0 = 180.0; // Latitude of right handside
};

class ProjectionPolarNorth : public Projection
{
public:
  glm::vec3 project (const glm::vec3 &) const override;
  int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const override;
  glm::mat4 getView (const glm::vec3 &, const float) const override;
  int getType () const override { return Projection::POLAR_NORTH; }
  bool setLon0 (const float &) const override { return false; }
};

class ProjectionPolarSouth : public Projection
{
public:
  glm::vec3 project (const glm::vec3 &) const override;
  int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const override;
  glm::mat4 getView (const glm::vec3 &, const float) const override;
  int getType () const override { return Projection::POLAR_SOUTH; }
  bool setLon0 (const float &) const override { return false; }
};

class ProjectionSet 
{
public:
  ProjectionSet () 
  {
    setup ();
  }
  ProjectionSet & operator= (const ProjectionSet & ps)
  {
    if (this != &ps)
      {
       setup ();
       current_ = ps.current_;  
      }
    return *this;
  }
  Projection * current () const
  {
    return proj[current_];
  }
  Projection * next ()
  {
    current_++; current_ %= 5; return current ();
  }
  void setType (const Projection::type & type)
  {
    current_ = type;
  }
  virtual std::string currentName () const
  {
 #define if_type(x) case Projection::x: return #x
     switch (current_)
       {
         if_type (XYZ);
         if_type (POLAR_NORTH);
         if_type (POLAR_SOUTH);
         if_type (MERCATOR);
         if_type (LATLON);
       }
#undef if_type
     return "";
  }
private:
  int current_ = Projection::XYZ;
  void setup ()
  { 
     proj[0] = &proj_xyz;
     proj[1] = &proj_polar_north;
     proj[2] = &proj_polar_south;
     proj[3] = &proj_mercator;
     proj[4] = &proj_latlon;
  }
  Projection * proj[5];
  ProjectionXYZ         proj_xyz;
  ProjectionPolarNorth proj_polar_north;
  ProjectionPolarSouth proj_polar_south;
  ProjectionMercator    proj_mercator;
  ProjectionLatLon      proj_latlon;
};



}
