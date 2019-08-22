#ifndef _GLGRIB_GSHHG_H
#define _GLGRIB_GSHHG_H


#include <vector>
#include <string>

namespace glgrib_gshhg
{
extern void read (const std::string &, int *, unsigned int *, 
		  std::vector<float> *, std::vector<unsigned int> *, 
                  const std::vector<unsigned int> & = {0x00000000}, 
                  const std::vector<unsigned int> & = {0x00000000});
};

#endif
