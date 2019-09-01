#include "glgrib_shapelib.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <shapefil.h>


#include <stdexcept>
#include <iostream>


void glgrib_shapelib::read (const std::string & path, int * numberOfPoints, 
		         unsigned int * numberOfLines, std::vector<float> * xyz,
			 std::vector<unsigned int> * ind)
{
  const float deg2rad = M_PI / 180.0f;


  SHPHandle fp = SHPOpen (path.c_str (), "r");

  if (fp == NULL)
    throw std::runtime_error (std::string ("Cannot open Shapefile data : ") + path);


  double minb[4], maxb[4];
  int ntype, nentities;
  SHPGetInfo (fp, &nentities, &ntype, minb, maxb);

  *numberOfLines = 0;
  *numberOfPoints = 0;

  int count = -1;
  for (int i = 0; i < nentities; i++)
    {

      SHPObject	* shape = SHPReadObject(fp, i);

      if (shape == NULL)
        throw std::runtime_error (std::string ("Cannot read Shapefile data : ") + path);
      
      for (int j = 0, ipart = 0; j < shape->nVertices; j++)
        {
          const char * type;
          bool newpart = false;

          if (j == 0 && shape->nParts > 0)
            {
              type = SHPPartTypeName (shape->panPartType[ipart]);
              ipart++;
              newpart = true;
            }
          
          if ((ipart < shape->nParts) && (shape->panPartStart[ipart] == j))
            {
              type = SHPPartTypeName (shape->panPartType[ipart]);
              ipart++;
              newpart = true;
            }


          float lon = shape->padfX[j] * deg2rad; 
          float lat = shape->padfY[j] * deg2rad;

          float coslon = cos (lon);
          float sinlon = sin (lon);
          float coslat = cos (lat);
          float sinlat = sin (lat);

          xyz->push_back (coslon * coslat);
          xyz->push_back (sinlon * coslat);
          xyz->push_back (         sinlat);

          (*numberOfPoints)++;

          if (newpart)
            {
              count++;
            }
          else
            {
              ind->push_back (count); count++;
              ind->push_back (count);
              (*numberOfLines)++;
            }


        }
    

      SHPDestroyObject (shape);


    }

  SHPClose (fp);

}

