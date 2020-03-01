#pragma once

#include <vector>

template <typename TYPE>
void glGribFitpolynomial
( 
  const std::vector<TYPE> &,
  const std::vector<TYPE> &,
  const int                ,
  std::vector<TYPE> &      
);

template <typename TYPE>
TYPE glgrib_evalpolynomial (const std::vector<TYPE> &, TYPE);


