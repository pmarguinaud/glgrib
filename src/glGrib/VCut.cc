#include "glGrib/VCut.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/OpenGL.h"
#include "glGrib/Contour.h"
#include "glGrib/Palette.h"

#include "shaders/include/vcut.h"

#include <stdio.h>

namespace glGrib
{

namespace
{

template <int N>
int intPow (int x)
{
  return x * intPow<N-1> (x);
}

template <>
int intPow<0> (int x)
{
  return 1;
}

const int bits = 16;
const int Nmax = intPow<bits> (2);

}

void VCut::render (const View & view, const OptionsLight & light) const
{
  if (! opts.on)
    return;

  if ((Nx < 0) || (Nz < 0))
    return;

  const auto & palette = this->palette;

  Program * program = Program::load ("VCUT");
  program->use (); 

  view.setMVP (program);
  palette.set (program);
  program->set ("Nx", Nx);
  program->set ("Nz", Nz);

  program->set ("valmin", meta.valmin);
  program->set ("valmax", meta.valmax);
  program->set ("palmin", palette.getMin ());
  program->set ("palmax", palette.getMax ());

  program->set ("dz", opts.height.uniform.dz);
  program->set ("luniformz", opts.height.uniform.on);
  program->set ("lconstantz", opts.height.constant.on);
  program->set ("scale0", opts.scale);
  program->set ("Nmax", Nmax-1);
    
  glDisable (GL_CULL_FACE);

  if (opts.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

  VAID.bind ();
  unsigned int ind[6] = {0, 1, 2, 1, 2, 3};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, (Nz - 1) * (Nx - 1));
  VAID.unbind ();

  if (opts.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

  glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void VCut::setupVertexAttributes () const
{
  lonlatbuffer->bind (GL_SHADER_STORAGE_BUFFER, vcutLonLat_idx);
  valuesbuffer->bind (GL_SHADER_STORAGE_BUFFER, vcutValues_idx);
  if (! opts.height.uniform.on)
    heightbuffer->bind (GL_SHADER_STORAGE_BUFFER, vcutHeight_idx);
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
    void init (const glm::vec3 & _xyz1, const glm::vec3 & _xyz2, const OptionsVCut & _opts)
    {
      xyz1 = _xyz1;
      xyz2 = _xyz2;
      glm::vec3 normal = glm::cross (xyz1, xyz2);
      normal = glm::normalize (normal);
      A = glm::inverse (glm::mat3 (xyz1, xyz2, normal));
      opts = &_opts;
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
      // Insert discontinuity marker
      if (first && (coords.size () > 0) && (! coords.back ().isNull ()))
        {
          if (opts->debug.on) printf (" first \n");
          coords.push_back (vcut_coords ());
	}

      // Interpolate point; use a linear approximation (easier & cheaper than real calculation)
      glm::vec3 xyz;
     
      if (opts->rough.on)
        {
          xyz = a > 0.5f ? xyzB : xyzA;
	}
      else
        {
          xyz = (1.0f - a) * xyzA + a * xyzB;
          xyz = xyz / glm::length (xyz);
        }

      // check vector is between xyz1 & xyz2
      glm::vec3 c12n = A * xyz;

      if ((c12n.x >= 0) && (c12n.y >= 0))
        {
          // First point : see whether starting/ending point of arc belong to this triangle
          if (first)
            look4end (xyzA, xyzB, xyzC, jgloA, jgloB, jgloC);
          if (opts->debug.on)
          printf (" %5d | %5d %5d | %12.4f | (%12.4f,%12.4f,%12.4f) - (%12.4f,%12.4f,%12.4f)\n",
                  static_cast<int>(coords.size ()), jgloA, jgloB, a, xyzA.x, xyzA.y, xyzA.z, xyzB.x, xyzB.y, xyzB.z);

	  // Distance to previous point
	  const float dx = [this,&xyz] ()
	  {
            if (coords.size () > 0)
              {
                const auto & XYZ = this->coords.back ().xyz;
	        return glm::distance (xyz, XYZ);
	      }
	    return 0.0f;
	  }();

	  // Current space is enough OR we already have skipped too many points
	  if ((dx >= lastDx * opts->contour.fracdx) || (skip > opts->contour.skipmax))
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
          // Close section, reset
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
      if (opts->debug.on) printf ("----------------------------------------------------------------------------------\n");

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
    // See whether the points starting/closing the arc belongs to the triangle
    // if so, add them to the list of points
    void look4end (const glm::vec3 & xyzA, const glm::vec3 & xyzB, const glm::vec3 & xyzC, 
		   const int jgloA, const int jgloB, const int jgloC) 
    {
      const glm::mat3 B = glm::inverse (glm::mat3 (xyzA, xyzB, xyzC));
      const auto c1ABC = B * xyz1;
      const auto c2ABC = B * xyz2;
      if ((c1ABC.x >= 0.0f) && (c1ABC.y >= 0.0f) && (c1ABC.z > 0.0f))
        {
          coords.push_back (vcut_coords (jgloA, jgloB, jgloC, -1.0f, xyz1));
          if (opts->debug.on) printf ("  Found #1 !\n");
	}
      if ((c2ABC.x >= 0.0f) && (c2ABC.y >= 0.0f) && (c2ABC.z > 0.0f))
        {
          coords.push_back (vcut_coords (jgloA, jgloB, jgloC, -1.0f, xyz2));
          if (opts->debug.on) printf ("  Found #2 !\n");
	}
    }
    glm::mat3 A;
    glm::vec3 xyz1, xyz2;
    std::vector<vcut_coords> coords;
    bool first;
    float lastDx = 0.0f;
    int skip = 0;
    const OptionsVCut * opts = nullptr;
  };

  // Start here
  opts = o;
  if (! opts.on)
    return;


  const float skip = 4 * 360.f;

  int n = std::min (opts.lon.size (), opts.lat.size ()), m = n;

  for (int i = 0; i < m; i++)
    if (opts.lat[i] > skip)
      n--;

  const int N = n-1;

  if (N < 1)
    {
      opts.on = false;
      return;
    }

  std::vector<float> lat1, lat2, lon1, lon2;

  lat1.reserve (N); lat2.reserve (N); lon1.reserve (N); lon2.reserve (N);

  for (int i = 0; i < m-1; i++)
    {
      const int i1 = i+0;
      const int i2 = i+1;
      const bool ok1 = opts.lat[i1] < skip;
      const bool ok2 = opts.lat[i2] < skip;
      if (ok1 && ok2)
        {
          lat1.push_back (opts.lat[i1] * deg2rad); lon1.push_back (opts.lon[i1] * deg2rad);
          lat2.push_back (opts.lat[i2] * deg2rad); lon2.push_back (opts.lon[i2] * deg2rad);
	}
    }


  std::vector<vcut_helper> isoh (N);

  bool dbg = opts.debug.on;

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

      glm::vec3 xyz1 = lonlat2xyz (lon1[n], lat1[n]);
      glm::vec3 xyz2 = lonlat2xyz (lon2[n], lat2[n]);
      glm::vec3 normal = glm::cross (xyz1, xyz2);

      float length = glm::length (normal);
      if (length < std::sin (deg2rad * 1.0f)) // 1 degree
        continue;

      normal = normal / length;
     
      isoh[n].init (xyz1, xyz2, opts);
      
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
      if (dbg) printf (" Nx = %d\n", Nx);
    }


  lonlatbuffer = OpenGLBufferPtr<float> (2 * Nx);
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

	  // Discontinuity : skip point
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

  Nz = opts.path.size ();
  valuesbuffer = OpenGLBufferPtr<float> (Nx * Nz);

  if (opts.height.constant.on)
    heightbuffer = OpenGLBufferPtr<float> (Nz);
  else if (! opts.height.uniform.on)
    heightbuffer = OpenGLBufferPtr<float> (Nx * Nz);
  else
    heightbuffer = OpenGLBufferPtr<float> (0);

  auto values = valuesbuffer->map ();
  auto height = heightbuffer->map ();

  bool heightgrid = (! opts.height.uniform.on) && (! opts.height.constant.on);

  OptionsMissing opts_missing;
  opts_missing.on = true;
  opts_missing.value = +std::numeric_limits<float>::max ();

  // Setup values & height
  for (int k = 0; k < Nz; k++)
    {
      FieldMetadata meta_k;

      BufferPtr<float> data_k;
      ld->load (&data_k, opts.path, opts_geom, k, &meta_k, 1, 0, false, opts_missing);
      const auto geometry_k = Geometry::load (ld, opts.path[k], opts_geom);

      if (k == 0)
        {
          meta = meta_k;
	}
      else
        {
          meta.valmin = std::min (meta_k.valmin, meta.valmin);
          meta.valmax = std::max (meta_k.valmax, meta.valmax);
        }

      if (! geometry->isEqual (*geometry_k))
        throw std::runtime_error (std::string ("Geometry mismatch"));

      if (opts.height.constant.on)
        {
          const int l = opts.height.constant.levels.size ();
	  if (k < l)
            {
              height[k] = opts.height.constant.levels[k];
	    }
	  else if (l >= 2)
            {
              height[k] = 2 * opts.height.constant.levels[l-1] - opts.height.constant.levels[l-2];
	    }
	  else
            {
              height[k] = k * 0.05f;
            }
	} 

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

	      // Discontinuity
              if (c.isNull ())
                {
                  values[j+i] = 0.0f;
                  if (heightgrid)
                    height[j+i] = 0.0f;
                }
              else
                {
                  float angle = std::acos (glm::dot (xyz1, c.xyz));
                  float x = angle / angle0;

		  // End point (begin or end arc)
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

		      if ((data_k[c.jgloA] == meta_k.valmis) || (data_k[c.jgloB] == meta_k.valmis) || (data_k[c.jgloC] == meta_k.valmis))
                        values[j+i] = meta_k.valmis;
		      else
                        values[j+i] = cABC.x * data_k[c.jgloA] + cABC.y * data_k[c.jgloB] + cABC.z * data_k[c.jgloC];

		    }
		  // Regular point
		  else
		    {
                      float aA, aB;
                      if (opts.rough.on)
                        {
                          aA = c.a > 0.5f ? 0.f : 1.f;
		          aB = c.a > 0.5f ? 1.f : 0.f;
			}
		      else
                        {
                          aA = 1.0f - c.a;
			  aB = c.a;
			}
		      if ((data_k[c.jgloA] == meta_k.valmis) || (data_k[c.jgloB] == meta_k.valmis))
                        values[j+i] = meta_k.valmis;
		      else
                        values[j+i] = aA * data_k[c.jgloA] + aB * data_k[c.jgloB];
		    }
                  if (heightgrid)
                    height[j+i] = z * (0.1f + (1.0f - x) * x);
                }
	    }
        }

    }

  palette = Palette (opts.palette, meta.valmin, meta.valmax);

#pragma omp parallel for 
  for (int i = 0; i < Nx * Nz; i++)
    {
      if (values[i] == meta.valmis)
        {
          values[i] = 0.0f;
	}
      else
        {
          float val = (values[i] - meta.valmin) / (meta.valmax - meta.valmin);
          values[i] = (1.0f + val * static_cast<float>(Nmax - 1)) / static_cast<float> (Nmax);
	}
    }


  setReady ();
}

}
