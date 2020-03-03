#pragma once

#include <vector>

namespace glGrib
{

template <typename TYPE>
void FitPolynomial
( 
  const std::vector<TYPE> &,
  const std::vector<TYPE> &,
  const int                ,
  std::vector<TYPE> &      
);

template <typename TYPE>
TYPE Evalpolynomial (const std::vector<TYPE> &, TYPE);



}
