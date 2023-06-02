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
  opts = o;
  if (! opts.on)
    return;

  float lon1 = deg2rad * opts.lon[0], lon2 = deg2rad * opts.lon[1];
  float lat1 = deg2rad * opts.lat[0], lat2 = deg2rad * opts.lat[1];

  glm::vec3 xyz1 = lonlat2xyz (lon1, lat1);
  glm::vec3 xyz2 = lonlat2xyz (lon2, lat2);

  glm::vec3 normal = glm::cross (xyz1, xyz2);

  const OptionsGeometry opts_geom;
  geometry = Geometry::load (ld, opts.path[0], opts_geom);

  BufferPtr<float> data;
  ld->load (&data, opts.path, opts_geom, 0, &meta);

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
    vcut_helper (const glm::vec3 & xyz1, const glm::vec3 & xyz2, const glm::vec3 & normal) 
      : A (glm::inverse (glm::mat3 (xyz1, xyz2, normal)))
    {
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
          printf (" first \n");
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

//    const auto & c0 = coords[0], & c1 = coords[n-1];
//    if ((c0.jgloA == c1.jgloA) && (c0.jgloB == c1.jgloB))
//      {
//        std::vector<vcut_coords> tmp = coords;
//      }

      coords.push_back (vcut_coords ());
      printf ("----------------------------------------------------------------------------------\n");

    }
    const std::vector<vcut_coords> & getCoords () const
    {
      return coords;
    }
  private:
    glm::mat3 A;
    std::vector<vcut_coords> coords;
    bool first;
  };

  vcut_helper isoh (xyz1, xyz2, normal);
  
  auto val = [&normal,this] (int jglo)
  {
    float lon, lat;
    this->geometry->index2latlon (jglo, &lat, &lon);
    glm::vec3 xyz = lonlat2xyz (lon, lat);
    return glm::dot (normal, xyz);
  };

  printf (" %5s | %5s %5s | %12s | (%12s,%12s,%12s) - (%12s,%12s,%12s)\n",
          "rank", "jgloA", "jgloB", "a", "xyzA.x", "xyzA.y", "xyzA.z", "xyzB.x", "xyzB.y", "xyzB.z");
  Contour::processTriangles (geometry, 0.0f, &isoh, val);

  const auto & coords = isoh.getCoords ();

  for (Nx = 0; Nx < static_cast<int>(coords.size ()); Nx++)
    if (coords[Nx].isNull ())
      break;

//Nx = 20;
  std::cout << " Nx = " << Nx << std::endl;

  Nz = 3;

  std::vector<float> lonlat (2 * Nx);

  printf (" (%12s,%12s)\n", "lon", "lat");
  for (int i = 0; i < Nx; i++)
    {
      float lon, lat;
      xyz2lonlat (coords[i].xyz, &lon, &lat);
      lonlat[2*i+0] = lon;
      lonlat[2*i+1] = lat;
      printf (" (%12.4f,%12.4f)\n", lon * rad2deg, lat * rad2deg);
    }    


  lonlatbuffer = OpenGLBufferPtr<float> (lonlat);
  valuesbuffer = OpenGLBufferPtr<float> (Nx * Nz);
  heightbuffer = OpenGLBufferPtr<float> (Nx * Nz);

  auto values = valuesbuffer->map ();
  auto height = heightbuffer->map ();

  float angle0 = std::acos (glm::dot (xyz1, xyz2));
  for (int iz = 0; iz < Nz; iz++)
    {
      float z = static_cast<float> (iz) / static_cast<float> (Nz - 1);
      for (int ix = 0; ix < Nx; ix++)
        {
//        float x = static_cast<float> (ix) / static_cast<float> (Nx - 1);
          float angle = std::acos (glm::dot (xyz1, coords[ix].xyz));
	  float x = angle / angle0;
          values[Nx*iz+ix] = x * z;
          height[Nx*iz+ix] = z * (0.1f + (1.0f - x) * x);
        }
    }

  setReady ();
}

}
