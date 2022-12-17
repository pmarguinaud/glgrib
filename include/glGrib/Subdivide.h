#pragma once

#include <vector>

#include <glm/gtc/type_ptr.hpp>

namespace glGrib
{

class Subdivide
{
public:

  Subdivide () {}

  void init (const std::vector<float> &, 
             const std::vector<unsigned int> &,
             int, int,
             int, int);
 
  void subdivide (float);

  void append (std::vector<float> &, std::vector<unsigned int> &, int, int);

  int getPtsLength () const
  {
    return xyz1.size () - xyz1_size;
  }

  int getTriLength () const
  {
    return ind1.size () - ind1_size;
  }

private:
  int indt1, indt2;
  int indp1, indp2;
  int indr1, indr2;
  std::vector<glm::vec3> xyz1;
  std::vector<unsigned int> ind1;
  int ind1_size;
  int xyz1_size;
  
};


}
