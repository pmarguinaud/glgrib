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

  Program * program = Program::load ("VCUT");
  program->use (); 

  view.setMVP (program);
  program->set ("Nx", Nx);
  program->set ("Nz", Nz);
  program->set ("colormax", glm::vec4 (1.0f, 0.0f, 0.0f, 1.0f));
  program->set ("colormin", glm::vec4 (0.0f, 0.0f, 1.0f, 1.0f));
  program->set ("valmin", 0.0f);
  program->set ("valmax", 1.0f);

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
    vcut_coords (int _jgloA, int _jgloB, float _a, const glm::vec3 & _xyz) 
      : jgloA (_jgloA), jgloB (_jgloB), a (_a), xyz (_xyz)
    {
    }
    vcut_coords () 
    {
    }
    bool isNull () const
    {
      return (jgloA < 0) && (jgloB < 0);
    }
    int jgloA = -1, jgloB = -1;
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
    }
    void push (const glm::vec3 & xyzA, const glm::vec3 & xyzB, 
               const int jgloA, const int jgloB, const float a)
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

      float lon, lat;
      xyz2lonlat (xyz, &lon, &lat);

      if ((c12n.x >= 0) && (c12n.y >= 0))
        {
          if (dbg)
          printf (" %5d | %5d %5d | %12.4f | (%12.4f,%12.4f,%12.4f) - (%12.4f,%12.4f,%12.4f)\n",
                  static_cast<int>(coords.size ()), jgloA, jgloB, a, xyzA.x, xyzA.y, xyzA.z, xyzB.x, xyzB.y, xyzB.z);
          coords.push_back (vcut_coords (jgloA, jgloB, a, xyz));
          first = false;
	}
      else
        {
          first = true;
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
    glm::mat3 A;
    glm::vec3 xyz1, xyz2;
    std::vector<vcut_coords> coords;
    bool first;
    bool dbg = false;
  };

  // Start here
  opts = o;
  if (! opts.on)
    return;

  const int N = std::min (opts.lon.size (), opts.lat.size ()) - 1;

  std::vector<vcut_helper> isoh (N);

  bool dbg = false;

  if (dbg)
  printf (" %5s | %5s %5s | %12s | (%12s,%12s,%12s) - (%12s,%12s,%12s)\n",
          "rank", "jgloA", "jgloB", "a", "xyzA.x", "xyzA.y", "xyzA.z", "xyzB.x", "xyzB.y", "xyzB.z");

  // Contouring on all sections
  for (int n = 0; n < N; n++)
    {
      if (dbg) printf (" n = %d\n", n);
      float lon1 = deg2rad * opts.lon[n], lon2 = deg2rad * opts.lon[n+1];
      float lat1 = deg2rad * opts.lat[n], lat2 = deg2rad * opts.lat[n+1];
     
      glm::vec3 xyz1 = lonlat2xyz (lon1, lat1);
      glm::vec3 xyz2 = lonlat2xyz (lon2, lat2);
      glm::vec3 normal = glm::cross (xyz1, xyz2);
     
      const OptionsGeometry opts_geom;
      geometry = Geometry::load (ld, opts.path[0], opts_geom);
     
      BufferPtr<float> data;
      ld->load (&data, opts.path, opts_geom, 0, &meta);
     
      isoh[n].init (xyz1, xyz2);
      
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
    }

  Nz = 3;

  lonlatbuffer = OpenGLBufferPtr<float> (2 * Nx);
  valuesbuffer = OpenGLBufferPtr<float> (Nx * Nz);
  heightbuffer = OpenGLBufferPtr<float> (Nx * Nz);

  auto values = valuesbuffer->map ();
  auto height = heightbuffer->map ();
  auto lonlat = lonlatbuffer->map ();

  if (dbg) printf (" (%12s,%12s)\n", "lon", "lat");
  for (int n = 0; n < N; n++)
    {
      const auto & coords = isoh[n].getCoords ();
      const auto & xyz1 = isoh[n].getXYZ1 ();
      const auto & xyz2 = isoh[n].getXYZ2 ();
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

      float angle0 = std::acos (glm::dot (xyz1, xyz2));
      for (int k = 0; k < Nz; k++)
        {
          float z = static_cast<float> (k) / static_cast<float> (Nz - 1);
          int j = Nx*k+Nx_offset[n];
          for (int i = 0; i < nx; i++)
            {
              if (coords[i].isNull ())
                {
                  values[j+i] = 0.0f;
                  height[j+i] = 0.0f;
                }
              else
                {
                  float angle = std::acos (glm::dot (xyz1, coords[i].xyz));
                  float x = angle / angle0;
                  values[j+i] = x * z;
                  height[j+i] = z * (0.1f + (1.0f - x) * x);
                }
	    }
        }

    }

  setReady ();
}

}
