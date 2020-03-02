#pragma once

#include <vector>

template <typename TYPE>
void glGribFitPolynomial
( 
  const std::vector<TYPE> &,
  const std::vector<TYPE> &,
  const int                ,
  std::vector<TYPE> &      
);

template <typename TYPE>
TYPE glGribEvalpolynomial (const std::vector<TYPE> &, TYPE);


