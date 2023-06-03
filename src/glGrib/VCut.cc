#include "glGrib/VCut.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/OpenGL.h"
#include "glGrib/Contour.h"

#include <stdio.h>

namespace glGrib
{

void VCut::render (const View & view, const OptionsLight & light) const
{
  if (! opts.on)
    return;

  if ((Nx < 0) || (Nz < 0))
    return;

  Program * program = Program::load ("VCUT");
  program->use (); 

  view.setMVP (program);
  program->set ("Nx", Nx);
  program->set ("Nz", Nz);
  program->set ("colormax", glm::vec4 (1.0f, 0.0f, 0.0f, 1.0f));
  program->set ("colormin", glm::vec4 (0.0f, 0.0f, 1.0f, 1.0f));
  program->set ("valmin", meta.valmin);
  program->set ("valmax", meta.valmax);

  glDisable (GL_CULL_FACE);
  glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

  VAID.bind ();
  unsigned int ind[6] = {0, 1, 2, 1, 2, 3};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, (Nz - 1) * (Nx - 1));
  VAID.unbind ();

  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
  glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void VCut::setupVertexAttributes () const
{
  lonlatbuffer->bind (GL_SHADER_STORAGE_BUFFER, 1);
  valuesbuffer->bind (GL_SHADER_STORAGE_BUFFER, 2);
  heightbuffer->bind (GL_SHADER_STORAGE_BUFFER, 3);
}

void VCut::setup (Loader * ld, const OptionsVCut & o)
{
  // Helper class for contouring
  class vcut_coords
  {
  public:
    vcut_coords (int _jgloA, int _jgloB, int _jgloC, float _a, const glm::vec3 & _xyz) 
      : jgloA (_jgloA), jgloB (_jgloB), jgloC (_jgloC), a (_a), xyz (_xyz)
    {
    }
    vcut_coords () 
    {
    }
    bool isNull () const
    {
      return (jgloA < 0) && (jgloB < 0);
    }
    int jgloA = -1, jgloB = -1, jgloC = -1;
    float a;
    glm::vec3 xyz;
  };

  class vcut_helper
  {
  public:
    vcut_helper () 
    {
    }
    void init (const glm::vec3 & _xyz1, const glm::vec3 & _xyz2, bool _dbg = false)
    {
      xyz1 = _xyz1;
      xyz2 = _xyz2;
      glm::vec3 normal = glm::cross (xyz1, xyz2);
      A = glm::inverse (glm::mat3 (xyz1, xyz2, normal));
      dbg = _dbg;
    }
    void start ()
    {
      first = true;
      lastDx = 0.0f;
      skip = 0;
    }
    void push (const glm::vec3 & xyzA, const glm::vec3 & xyzB, const glm::vec3 & xyzC, 
               const int jgloA, const int jgloB, const int jgloC, const float a)
    {
      if (first && (coords.size () > 0) && (! coords.back ().isNull ()))
        {
          if (dbg) printf (" first \n");
          coords.push_back (vcut_coords ());
	}

      glm::vec3 xyz = (1.0f - a) * xyzA + a * xyzB;

      xyz = xyz / glm::length (xyz);

      // check vector is between xyz1 & xyz2
      glm::vec3 c12n = A * xyz;

      if ((c12n.x >= 0) && (c12n.y >= 0))
        {
          if (first)
            look4end (xyzA, xyzB, xyzC, jgloA, jgloB, jgloC);
          if (dbg)
          printf (" %5d | %5d %5d | %12.4f | (%12.4f,%12.4f,%12.4f) - (%12.4f,%12.4f,%12.4f)\n",
                  static_cast<int>(coords.size ()), jgloA, jgloB, a, xyzA.x, xyzA.y, xyzA.z, xyzB.x, xyzB.y, xyzB.z);

	  const float dx = [this,&xyz] ()
	  {
            if (coords.size () > 0)
              {
                const auto & XYZ = this->coords.back ().xyz;
	        return glm::distance (xyz, XYZ);
	      }
	    return 0.0f;
	  }();

	  if ((dx >= lastDx * fracDx) || (skip > skipMax))
            {
              coords.push_back (vcut_coords (jgloA, jgloB, jgloC, a, xyz));
              lastDx = dx;
	      skip = 0;
	    }
	  else
            {
              skip++;
	    }

          first = false;
	}
      else
        {
          if (! first)
            look4end (xyzA, xyzB, xyzC, jgloA, jgloB, jgloC);
          start ();
	}

    }
    void close (bool)
    {
      size_t n = coords.size ();
      if ((n > 0) && (coords[n-1].jgloA < 0))
        return;
      coords.push_back (vcut_coords ());
      if (dbg) printf ("----------------------------------------------------------------------------------\n");

    }
    const std::vector<vcut_coords> & getCoords () const
    {
      return coords;
    }
    const glm::vec3 & getXYZ1 () const
    {
      return xyz1;
    }
    const glm::vec3 & getXYZ2 () const
    {
      return xyz2;
    }
  private:
    void look4end (const glm::vec3 & xyzA, const glm::vec3 & xyzB, const glm::vec3 & xyzC, int jgloA, int jgloB, int jgloC) 
    {
      const glm::mat3 B = glm::inverse (glm::mat3 (xyzA, xyzB, xyzC));
      const auto c1ABC = B * xyz1;
      const auto c2ABC = B * xyz2;
      if ((c1ABC.x >= 0.0f) && (c1ABC.y >= 0.0f) && (c1ABC.z > 0.0f))
        {
          coords.push_back (vcut_coords (jgloA, jgloB, jgloC, -1.0f, xyz1));
          if (dbg) printf ("  Found #1 !\n");
	}
      if ((c2ABC.x >= 0.0f) && (c2ABC.y >= 0.0f) && (c2ABC.z > 0.0f))
        {
          coords.push_back (vcut_coords (jgloA, jgloB, jgloC, -1.0f, xyz2));
          if (dbg) printf ("  Found #2 !\n");
	}
    }
    glm::mat3 A;
    glm::vec3 xyz1, xyz2;
    std::vector<vcut_coords> coords;
    bool first;
    bool dbg = false;
    float fracDx = 0.4f;
    float lastDx = 0.0f;
    int skip = 0;
    int skipMax = 3;
  };

  // Start here
  opts = o;
  if (! opts.on)
    return;

  const int N = std::min (opts.lon.size (), opts.lat.size ()) - 1;

  if (N < 2)
    {
      opts.on = false;
      return;
    }

  std::vector<vcut_helper> isoh (N);

  bool dbg = true;

  if (dbg)
  printf (" %5s | %5s %5s | %12s | (%12s,%12s,%12s) - (%12s,%12s,%12s)\n",
          "rank", "jgloA", "jgloB", "a", "xyzA.x", "xyzA.y", "xyzA.z", "xyzB.x", "xyzB.y", "xyzB.z");

  const OptionsGeometry opts_geom;
  geometry = Geometry::load (ld, opts.path[0], opts_geom);
  
  // Contouring on all sections
#pragma omp parallel for if (! dbg)
  for (int n = 0; n < N; n++)
    {
      if (dbg) printf (" n = %d\n", n);
      float lon1 = deg2rad * opts.lon[n], lon2 = deg2rad * opts.lon[n+1];
      float lat1 = deg2rad * opts.lat[n], lat2 = deg2rad * opts.lat[n+1];
     
      glm::vec3 xyz1 = lonlat2xyz (lon1, lat1);
      glm::vec3 xyz2 = lonlat2xyz (lon2, lat2);
      glm::vec3 normal = glm::cross (xyz1, xyz2);
     
      isoh[n].init (xyz1, xyz2, dbg);
      
      // Scalar product with normal vector : subdivide the sphere in two parts
      auto val = [&normal,this] (int jglo)
      {
        float lon, lat;
        this->geometry->index2latlon (jglo, &lat, &lon);
        glm::vec3 xyz = lonlat2xyz (lon, lat);
        return glm::dot (normal, xyz);
      };

      Contour::processTriangles (geometry, 0.0f, &isoh[n], val);
    }

  int Nx_offset[N];
  Nx = 0;
  for (int n = 0; n < N; n++)
    {
      Nx_offset[n] = Nx;
      Nx += isoh[n].getCoords ().size ();
      printf (" Nx = %d\n", Nx);
    }

  Nz = 3;

  lonlatbuffer = OpenGLBufferPtr<float> (2 * Nx);
  valuesbuffer = OpenGLBufferPtr<float> (Nx * Nz);
  heightbuffer = OpenGLBufferPtr<float> (Nx * Nz);

  auto values = valuesbuffer->map ();
  auto height = heightbuffer->map ();
  auto lonlat = lonlatbuffer->map ();

  if (dbg) printf (" (%12s,%12s)\n", "lon", "lat");

  // Setup coordinates
#pragma omp parallel for if (! dbg)
  for (int n = 0; n < N; n++)
    {
      const auto & coords = isoh[n].getCoords ();
      const int nx = coords.size ();
      for (int i = 0; i < nx; i++)
        {
          int j = 2*(Nx_offset[n]+i);
          if (coords[i].isNull ())
            {
              lonlat[j+0] = 0.0f;
              lonlat[j+1] = 2.0f * pi;
            }
          else
            {
              float lon, lat;
              xyz2lonlat (coords[i].xyz, &lon, &lat);
              lonlat[j+0] = lon;
              lonlat[j+1] = lat;
              if (dbg) printf (" (%12.4f,%12.4f)\n", lon * rad2deg, lat * rad2deg);
            }    
	}
     }

  BufferPtr<float> data;
  ld->load (&data, opts.path, opts_geom, 0, &meta);
     
  // Setup values & height
  for (int k = 0; k < Nz; k++)
    {
      float z = static_cast<float> (k) / static_cast<float> (Nz - 1);
#pragma omp parallel for if (! dbg)
      for (int n = 0; n < N; n++)
        {
          const auto & coords = isoh[n].getCoords ();
          const auto & xyz1 = isoh[n].getXYZ1 ();
          const auto & xyz2 = isoh[n].getXYZ2 ();
          const int nx = coords.size ();
     
          float angle0 = std::acos (glm::dot (xyz1, xyz2));
          int j = Nx*k+Nx_offset[n];
          for (int i = 0; i < nx; i++)
            {
              const auto & c = coords[i];
              if (c.isNull ())
                {
                  values[j+i] = 0.0f;
                  height[j+i] = 0.0f;
                }
              else
                {
                  float angle = std::acos (glm::dot (xyz1, c.xyz));
                  float x = angle / angle0;
                  if (c.a < 0.0f)
                    {
                      auto jglo2xyz = [this] (const int jglo)
		      {
                        float lat, lon;
                        this->geometry->index2latlon (jglo, &lat, &lon);
                        return lonlat2xyz (lon, lat);
		      };
		      const auto xyzA = jglo2xyz (c.jgloA);
		      const auto xyzB = jglo2xyz (c.jgloB);
		      const auto xyzC = jglo2xyz (c.jgloC);
                      const auto B = glm::inverse (glm::mat3 (xyzA, xyzB, xyzC));
                      auto cABC = B * c.xyz;
		      cABC = cABC / (cABC.x + cABC.y + cABC.z);
                      values[j+i] = cABC.x * data[c.jgloA] + cABC.y * data[c.jgloB] + cABC.z * data[c.jgloC];
		    }
		  else
		    {
                      values[j+i] = (1.0f - c.a) * data[c.jgloA] + c.a * data[c.jgloB];
		    }
                  height[j+i] = z * (0.1f + (1.0f - x) * x);
                }
	    }
        }

    }

  setReady ();
}

}
