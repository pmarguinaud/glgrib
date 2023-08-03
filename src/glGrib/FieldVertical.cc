#include "glGrib/FieldVertical.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/OpenGL.h"
#include "glGrib/Contour.h"
#include "glGrib/Palette.h"

#include "shaders/include/vertical.h"

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

FieldVertical * FieldVertical::clone () const
{
  FieldVertical * fld = new FieldVertical (Field::Privatizer ());
  *fld = *this;
  return fld;
}

void FieldVertical::render (const View & view, const OptionsLight & light) const
{
  if ((Nx < 0) || (Nz < 0))
    return;

  const auto & palette = this->palette;

  Program * program = Program::load ("VERTICAL");
  program->use (); 

  view.setMVP (program);
  palette.set (program);
  program->set ("Nx", Nx);
  program->set ("Nz", Nz);

  program->set ("valmin", meta[0].valmin);
  program->set ("valmax", meta[0].valmax);
  program->set ("palmin", palette.getMin ());
  program->set ("palmax", palette.getMax ());

  program->set ("dz", opts.vertical.height.uniform.dz);
  program->set ("luniformz", opts.vertical.height.uniform.on);
  program->set ("lconstantz", opts.vertical.height.constant.on);
  program->set ("scale0", opts.scale);
  program->set ("Nmax", Nmax-1);
    
  glDisable (GL_CULL_FACE);

  if (opts.vertical.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

  unsigned int ind[6] = {0, 1, 2, 1, 2, 3};

  d.VAID.bind ();

  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, (Nz - 1) * (Nx - 1));

  d.VAID.unbind ();

  if (opts.vertical.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

  glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void FieldVertical::vertical_vaid::setupVertexAttributes () const
{
  lonlatbuffer->bind (GL_SHADER_STORAGE_BUFFER, verticalLonLat_idx);
  valuesbuffer->bind (GL_SHADER_STORAGE_BUFFER, verticalValues_idx);
  if (heightbuffer->size () > 0)
    heightbuffer->bind (GL_SHADER_STORAGE_BUFFER, verticalHeight_idx);
}

void FieldVertical::setup (const Field::Privatizer, Loader * ld, const OptionsField & o)
{

  // Helper class for contouring
  class vertical_coords
  {
  public:
    vertical_coords (int _jgloA, int _jgloB, int _jgloC, float _a, const glm::vec3 & _xyz) 
      : jgloA (_jgloA), jgloB (_jgloB), jgloC (_jgloC), a (_a), xyz (_xyz)
    {
    }
    vertical_coords () 
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

  class vertical_helper
  {
  public:
    vertical_helper () 
    {
    }
    void init (const glm::vec3 & _xyz1, const glm::vec3 & _xyz2, const OptionsVertical & _opts)
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
          coords.push_back (vertical_coords ());
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
              coords.push_back (vertical_coords (jgloA, jgloB, jgloC, a, xyz));
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
      coords.push_back (vertical_coords ());
      if (opts->debug.on) printf ("----------------------------------------------------------------------------------\n");

    }
    const std::vector<vertical_coords> & getCoords () const
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
          coords.push_back (vertical_coords (jgloA, jgloB, jgloC, -1.0f, xyz1));
          if (opts->debug.on) printf ("  Found #1 !\n");
	}
      if ((c2ABC.x >= 0.0f) && (c2ABC.y >= 0.0f) && (c2ABC.z > 0.0f))
        {
          coords.push_back (vertical_coords (jgloA, jgloB, jgloC, -1.0f, xyz2));
          if (opts->debug.on) printf ("  Found #2 !\n");
	}
    }
    glm::mat3 A;
    glm::vec3 xyz1, xyz2;
    std::vector<vertical_coords> coords;
    bool first;
    float lastDx = 0.0f;
    int skip = 0;
    const OptionsVertical * opts = nullptr;
  };

  // Start here
  opts = o;

  const float skip = 4 * 360.f;

  const int n = std::min (opts.vertical.lon.size (), opts.vertical.lat.size ());

  meta.resize (1);

  std::vector<float> lat1, lat2, lon1, lon2;

  lat1.reserve (n); lat2.reserve (n); lon1.reserve (n); lon2.reserve (n);

  for (int i = 0; i < n-1; i++)
    {
      const int i1 = i+0;
      const int i2 = i+1;
      const bool ok1 = opts.vertical.lat[i1] < skip;
      const bool ok2 = opts.vertical.lat[i2] < skip;
      if (ok1 && ok2)
        {
          lat1.push_back (opts.vertical.lat[i1] * deg2rad); lon1.push_back (opts.vertical.lon[i1] * deg2rad);
          lat2.push_back (opts.vertical.lat[i2] * deg2rad); lon2.push_back (opts.vertical.lon[i2] * deg2rad);
	}
    }

  const int N = lat1.size ();

  if (N < 1)
    return;

  std::vector<vertical_helper> isoh (N);

  bool dbg = opts.vertical.debug.on;

  if (dbg)
  {
  printf (" N = %d\n", N);
  printf (" %5s | %5s %5s | %12s | (%12s,%12s,%12s) - (%12s,%12s,%12s)\n",
          "rank", "jgloA", "jgloB", "a", "xyzA.x", "xyzA.y", "xyzA.z", "xyzB.x", "xyzB.y", "xyzB.z");
  }

  const OptionsGeometry opts_geom;
  auto geometry = Geometry::load (ld, opts.path[0], opts_geom);
  setGeometry (geometry);
  
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
     
      isoh[n].init (xyz1, xyz2, opts.vertical);
      
      // Scalar product with normal vector : subdivide the sphere in two parts
      auto val = [&normal,&geometry,this] (int jglo)
      {
        float lon, lat;
        geometry->index2latlon (jglo, &lat, &lon);
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


  d.lonlatbuffer = OpenGLBufferPtr<float> (2 * Nx);
  auto lonlat = d.lonlatbuffer->map ();

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
  d.valuesbuffer = OpenGLBufferPtr<float> (Nx * Nz);

  if (opts.vertical.height.constant.on)
    d.heightbuffer = OpenGLBufferPtr<float> (Nz);
  else if (! opts.vertical.height.uniform.on)
    d.heightbuffer = OpenGLBufferPtr<float> (Nx * Nz);
  else
    d.heightbuffer = OpenGLBufferPtr<float> (0);

  auto values = d.valuesbuffer->map ();
  auto height = d.heightbuffer->map ();

  bool heightgrid = (! opts.vertical.height.uniform.on) && (! opts.vertical.height.constant.on);

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
          meta[0] = meta_k;
	}
      else
        {
          meta[0].valmin = std::min (meta_k.valmin, meta[0].valmin);
          meta[0].valmax = std::max (meta_k.valmax, meta[0].valmax);
        }

      if (! geometry->isEqual (*geometry_k))
        throw std::runtime_error (std::string ("Geometry mismatch"));

      if (opts.vertical.height.constant.on)
        {
          const int l = opts.vertical.height.constant.levels.size ();
	  if (k < l)
            {
              height[k] = opts.vertical.height.constant.levels[k];
	    }
	  else if (l >= 2)
            {
              height[k] = 2 * opts.vertical.height.constant.levels[l-1] - opts.vertical.height.constant.levels[l-2];
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
                      auto jglo2xyz = [&geometry,this] (const int jglo)
		      {
                        float lat, lon;
                        geometry->index2latlon (jglo, &lat, &lon);
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
                      if (opts.vertical.rough.on)
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

  palette = Palette (opts.palette, meta[0].valmin, meta[0].valmax);

#pragma omp parallel for 
  for (int i = 0; i < Nx * Nz; i++)
    {
      if (values[i] == meta[0].valmis)
        {
          values[i] = 0.0f;
	}
      else
        {
          float val = (values[i] - meta[0].valmin) / (meta[0].valmax - meta[0].valmin);
          values[i] = (1.0f + val * static_cast<float>(Nmax - 1)) / static_cast<float> (Nmax);
	}
    }


  setReady ();
}

}
