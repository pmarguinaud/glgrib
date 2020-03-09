#include "glGribFieldScalar.h"
#include "glGribTrigonometry.h"
#include "glGribProgram.h"
#include "glGribPalette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

glGrib::FieldScalar::FieldScalar (const glGrib::FieldScalar & field)
{
  if (field.isReady ())
    {
      // Cleanup already existing VAOs
      clear ();
      operator= (field);
    }
}

void glGrib::FieldScalar::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayIDpoints);
  glGrib::Field::clear ();
}


glGrib::FieldScalar * glGrib::FieldScalar::clone () const
{
  if (this == nullptr)
    return nullptr;
  glGrib::FieldScalar * fld = new glGrib::FieldScalar ();
  *fld = *this;
  return fld;
}

glGrib::FieldScalar & glGrib::FieldScalar::operator= (const glGrib::FieldScalar & field)
{
  if (this != &field)
    {
      clear ();
      if (field.isReady ())
        {
          glGrib::Field::operator= (field);
          setupVertexAttributes ();
          setReady ();
        }
    }
  return *this;
}

void glGrib::FieldScalar::setupVertexAttributes ()
{
  switch (opts.scalar.pack.bits)
    {
      case  8:
        setupVertexAttributes <unsigned char > ();
      break;
      case 16:
        setupVertexAttributes <unsigned short> ();
      break;
      case 32:
        setupVertexAttributes <unsigned int  > ();
      break;
      default:
        throw std::runtime_error (std::string ("Wrong number of bits for packing field: ") +
                                  std::to_string (opts.scalar.pack.bits));
    }
}

template <typename T>
void glGrib::FieldScalar::setupVertexAttributes ()
{
  numberOfPoints = geometry->getNumberOfPoints ();
  numberOfTriangles = geometry->getNumberOfTriangles ();

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  geometry->bindCoordinates (0);

  colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 

  glVertexAttribPointer (1, 1, getOpenGLType<T> (), GL_TRUE, 0,  nullptr); 

  geometry->bindTriangles ();

  bindHeight <T> (2);

  if (opts.mpiview.on)
    {
      mpivbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (3); 
      glVertexAttribPointer (3, 3, GL_FLOAT, GL_FALSE, 0, nullptr); 
//    glVertexAttribDivisor (3, 1);
    }
  else
    {
      glDisableVertexAttribArray (3);
      glVertexAttrib3f (3, 0.0f, 0.0f, 0.0f);
    }


  glBindVertexArray (0); 

  // Points
  glGenVertexArrays (1, &VertexArrayIDpoints);
  glBindVertexArray (VertexArrayIDpoints);

  geometry->bindCoordinates (0);
  glVertexAttribDivisor (0, 1);
  
  colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, 1, getOpenGLType<T> (), GL_TRUE, 0, nullptr); 
  glVertexAttribDivisor (1, 1);

  if (heightbuffer)
    {
      heightbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (2);
      glVertexAttribPointer (2, 1, getOpenGLType<T> (), GL_TRUE, 0, nullptr);
      glVertexAttribDivisor (2, 1);
    }
  else
    {
      glDisableVertexAttribArray (2);
      glVertexAttrib1f (2, 0.0f);
    }


  glBindVertexArray (0); 

  if (opts.geometry.frame.on)
    setupVertexAttributesFrame ();

}

void glGrib::FieldScalar::setup (glGrib::Loader * ld, const glGrib::OptionsField & o, float slot)
{
  opts = o;
  switch (opts.scalar.pack.bits)
    {
      case  8:
        setup<unsigned char > (ld, o, slot);
      break;
      case 16:
        setup<unsigned short> (ld, o, slot);
      break;
      case 32:
        setup<unsigned int  > (ld, o, slot);
      break;
      default:
        throw std::runtime_error (std::string ("Wrong number of bits for packing field: ") +
                                  std::to_string (opts.scalar.pack.bits));
    }
}

template <typename T>
void glGrib::FieldScalar::setup (glGrib::Loader * ld, const glGrib::OptionsField & o, float slot)
{
  opts = o;

  glGrib::FieldMetadata meta1;

  glGrib::FieldFloatBufferPtr data;
  ld->load (&data, opts.path, opts.geometry, slot, &meta1, 1, 0, opts.diff.on);
  meta.push_back (meta1);

  palette = glGrib::Palette::create (opts.palette, getNormedMinValue (), getNormedMaxValue ());

  geometry = glGrib::Geometry::load (ld, opts.path[int (slot)], opts.geometry);

  if (opts.hilo.on)
    setupHilo (data);

  colorbuffer = newGlgribOpenGLBufferPtr (geometry->getNumberOfPoints () * sizeof (T));
  T * col = (T *)colorbuffer->map ();
  pack<T>  (data->data (), geometry->getNumberOfPoints (), meta1.valmin, 
            meta1.valmax, meta1.valmis, col);
  col = nullptr;
  colorbuffer->unmap ();

  loadHeight <T> (colorbuffer, ld);

  if (opts.mpiview.on)
    setupMpiView (ld, o, slot);

  setupVertexAttributes<T> ();

  if (opts.no_value_pointer.on)
    values.push_back (newGlgribFieldFloatBufferPtr ((float*)nullptr));
  else
    values.push_back (data);

  setReady ();
}

void glGrib::FieldScalar::setupMpiView (glGrib::Loader * ld, const glGrib::OptionsField & o, float slot)
{
  int size = geometry->getNumberOfPoints ();

  glGrib::FieldMetadata mpiview_meta;
  glGrib::FieldFloatBufferPtr mpiview;

  ld->load (&mpiview, opts.mpiview.path, opts.geometry, slot, &mpiview_meta, 1, 0);

  float * data = mpiview->data ();

  float pmin = *std::min_element (data, data + size);
  float pmax = *std::max_element (data, data + size);

  int min = (int)pmin;
  int max = (int)pmax;

  glm::vec3 Disp[max];
  glm::vec2 Disl[max];
  int count[max];

  for (int i = 0; i < max; i++)
    {
      Disp[i] = glm::vec3 (0.0f, 0.0f, 0.0f);
      count[i] = 0;
    }

  for (int i = 0; i < size; i++)
    {
      float lon, lat;
      geometry->index2latlon (i, &lat, &lon);

      int j = (int)(*mpiview)[i]-1;

      Disp[j] += lonlat2xyz (glm::vec2 (lon, lat));
      count[j]++;
    }

  for (int i = 0; i < max; i++)
    Disl[i] = xyz2lonlat (glm::normalize (Disp[i] / (float)count[i]));

  if(0)
  for (int i = 0; i < max; i++)
    printf (" %8d | %12.2f, %12.2f\n", i, rad2deg * Disl[i].x, rad2deg * Disl[i].y);

  mpivbuffer = newGlgribOpenGLBufferPtr (3 * size * sizeof (float));
  float * mpiv = (float *)mpivbuffer->map ();
  for (int i = 0; i < size; i++)
    {
      int j = (*mpiview)[i]-1;
      mpiv[3*i+0] = (*mpiview)[i];
      mpiv[3*i+1] = Disl[j].x;
      mpiv[3*i+2] = Disl[j].y;
    }
  mpiv = nullptr;
  mpivbuffer->unmap ();

}

void glGrib::FieldScalar::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  float scale0 = opts.scale;

  if (opts.mpiview.on)
    scale0 = scale0 / (1.0f + opts.mpiview.scale);

  glGrib::Program * program = glGrib::Program::load (opts.scalar.points.on 
                                                 ? glGrib::Program::SCALAR_POINTS 
                                                 : glGrib::Program::SCALAR);

  program->use ();


  view.setMVP (program);
  program->set (light);
  palette.setRGBA255 (program->programID);
  program->set ("scale0", scale0);
  program->set ("valmin", getNormedMinValue ());
  program->set ("valmax", getNormedMaxValue ());
  program->set ("palmin", palette.getMin ());
  program->set ("palmax", palette.getMax ());
  program->set ("height_scale", opts.geometry.height.scale);
  program->set ("discrete", opts.scalar.discrete.on);
  program->set ("mpiview_scale", opts.mpiview.on ? opts.mpiview.scale : 0.0f);

  program->set ("RGBAM", opts.scalar.discrete.missing_color);
 
  geometry->setProgramParameters (program);

  int Nmax = 1;
  for (int i = 0; i < opts.scalar.pack.bits; i++)
    Nmax = Nmax * 2;

  program->set ("Nmax", Nmax-1);
    
  if (opts.scalar.points.on)
    {
      float length = view.pixelToDistAtNadir (10);
    
      program->set ("length10", length);
      program->set ("pointSiz", opts.scalar.points.size.value);
      program->set ("lpointZoo", opts.scalar.points.size.variable.on);
      program->set ("factor", opts.scalar.points.size.factor.on);
    
      glBindVertexArray (VertexArrayIDpoints);
    
      unsigned int ind[6] = {0, 1, 2, 2, 3, 0}; 
      glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, numberOfPoints);
    
      glBindVertexArray (0);
    }
  else
    {
      program->set ("smoothed", opts.scalar.smooth.on);
    
      if (opts.scalar.wireframe.on)
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    
      glBindVertexArray (VertexArrayID);

      geometry->renderTriangles ();
      
      glBindVertexArray (0);

      if (opts.scalar.wireframe.on)
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    }

  view.delMVP (program);

  renderHilo (view);
  
  if (opts.geometry.frame.on)
    renderFrame (view);

}

glGrib::FieldScalar::~FieldScalar ()
{
}


