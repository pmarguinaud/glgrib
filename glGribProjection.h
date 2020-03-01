#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

class glGribProjection
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

class glGribProjectionXyz : public glGribProjection
{
public:
  glm::vec3 project (const glm::vec3 &) const override;
  int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const override;
  glm::mat4 getView (const glm::vec3 &, const float) const override;
  int getType () const override { return glGribProjection::XYZ; }
  bool setLon0 (const float &) const override { return false; }
};

class glGribProjectionLatlon : public glGribProjection
{
public:
  glm::vec3 project (const glm::vec3 &) const override;
  int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const override;
  glm::mat4 getView (const glm::vec3 &, const float) const override;
  int getType () const override { return glGribProjection::LATLON; }
  bool setLon0 (const float & lon) const override { lon0 = lon; return true; }
private:
  mutable float lon0 = 180.0; // Latitude of right handside
};

class glGribProjectionMercator : public glGribProjection
{
public:
  glm::vec3 project (const glm::vec3 &) const override;
  int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const override;
  glm::mat4 getView (const glm::vec3 &, const float) const override;
  int getType () const override { return glGribProjection::MERCATOR; }
  bool setLon0 (const float & lon) const override { lon0 = lon; return true; }
private:
  mutable float lon0 = 180.0; // Latitude of right handside
};

class glGribProjectionPolarNorth : public glGribProjection
{
public:
  glm::vec3 project (const glm::vec3 &) const override;
  int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const override;
  glm::mat4 getView (const glm::vec3 &, const float) const override;
  int getType () const override { return glGribProjection::POLAR_NORTH; }
  bool setLon0 (const float &) const override { return false; }
};

class glGribProjectionPolarSouth : public glGribProjection
{
public:
  glm::vec3 project (const glm::vec3 &) const override;
  int unproject (const glm::vec3 &, const glm::vec3 &, glm::vec3 *) const override;
  glm::mat4 getView (const glm::vec3 &, const float) const override;
  int getType () const override { return glGribProjection::POLAR_SOUTH; }
  bool setLon0 (const float &) const override { return false; }
};

class glGribProjectionSet 
{
public:
  glGribProjectionSet () 
  {
    setup ();
  }
  glGribProjectionSet & operator= (const glGribProjectionSet & ps)
  {
    setup ();
    current_ = ps.current_;
    return *this;
  }
  glGribProjection * current () const
  {
    return proj[current_];
  }
  glGribProjection * next ()
  {
    current_++; current_ %= 5; return current ();
  }
  void setType (const glGribProjection::type & type)
  {
    current_ = type;
  }
  virtual std::string currentName () const
  {
 #define if_type(x) case glGribProjection::x: return #x
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
  int current_ = glGribProjection::XYZ;
  void setup ()
  { 
     proj[0] = &proj_xyz;
     proj[1] = &proj_polar_north;
     proj[2] = &proj_polar_south;
     proj[3] = &proj_mercator;
     proj[4] = &proj_latlon;
  }
  glGribProjection * proj[5];
  glGribProjectionXyz         proj_xyz;
  glGribProjectionPolarNorth proj_polar_north;
  glGribProjectionPolarSouth proj_polar_south;
  glGribProjectionMercator    proj_mercator;
  glGribProjectionLatlon      proj_latlon;
};


