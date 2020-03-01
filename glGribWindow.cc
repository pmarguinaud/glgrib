#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glGribWindow.h"
#include "glGribPng.h"
#include "glGribFieldScalar.h"
#include "glGribFieldVector.h"

#include <iostream>
#include <stdexcept>

static double currentTime ()
{
  struct timeval tv;
  struct timezone tz;
  gettimeofday (&tv, &tz);
  return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

static
void APIENTRY debugCallback (unsigned int source, unsigned int type, GLuint id, unsigned int severity, 
                              int length, const char * message, const void * data)
{
  glGribWindow * gwindow = (glGribWindow *)data;
  gwindow->debug (source, type, id, severity, length, message);
}

static 
void cursorPositionCallback (GLFWwindow * window, double xpos, double ypos)
{
  glGribWindow * gwindow = (glGribWindow *)glfwGetWindowUserPointer (window);
  gwindow->displayCursorPosition (xpos, ypos);
}

static
void mouseButtonCallback (GLFWwindow * window, int button, int action, int mods)
{
  glGribWindow * gwindow = (glGribWindow *)glfwGetWindowUserPointer (window);
  gwindow->onclick (button, action, mods);
}

static
void scroll_callback (GLFWwindow * window, double xoffset, double yoffset)
{
  glGribWindow * gwindow = (glGribWindow *)glfwGetWindowUserPointer (window);
  gwindow->scroll (xoffset, yoffset);
}

static 
void resize_callback (GLFWwindow * window, int width, int height)
{
  glGribWindow * gwindow = (glGribWindow *)glfwGetWindowUserPointer (window);
  gwindow->resize (width, height);
}

static 
void keyCallback (GLFWwindow * window, int key, int scancode, int action, int mods)
{
  glGribWindow * gwindow = (glGribWindow *)glfwGetWindowUserPointer (window);
  gwindow->onkey (key, scancode, action, mods);
}

void glGribWindow::showHelpItem (const char * mm, const char * k, const char * desc, const char * action)
{
  char line[strlen (desc) + strlen (action) + 64];
  char key[32];
  key[0] = '\0';
  if (strcmp (mm, "NONE") != 0)
    {
      strcpy (key, mm);
      strcat (key, "-");
    }
  strcat (key, k);

  sprintf (line, "%-32s %64s\n", key, strlen (desc) ? desc : action);

  int len = strlen (line);
  for (int i = 0; i < len; i++)
    if (line[i] == ' ')
      line[i] = '.';
  printf ("%s", line);
}

void glGribWindow::onkey (int key, int scancode, int action, int mods, bool help)
{

  if ((action == GLFW_PRESS || action == GLFW_REPEAT) || help)
    {
      glGribWindowIfKey (NONE,    PAGE_UP     ,  One field forward,  next = true);
      glGribWindowIfKey (NONE,    PAGE_DOWN   ,  One field backward, prev = true);
      glGribWindowIfKey (NONE,    T     ,  Hide/show location & field value at cursor position  , toggle_cursorpos_display ());
      glGribWindowIfKey (NONE,    TAB   ,  Enable/disable earth rotation                        , toggle_rotate            ());
      glGribWindowIfKey (CONTROL, TAB   ,  Enable/disable light rotation                        , toggle_rotate_light      ());
      glGribWindowIfKey (NONE,    Y     ,  Display landscape or current field as wireframe      , toggle_wireframe         ());
      glGribWindowIfKey (NONE,    D     ,  Use a framebuffer and generate a snapshot            , framebuffer              ());
      glGribWindowIfKey (NONE,    W     ,  Increase field of view                               , widen_fov                ());
      glGribWindowIfKey (NONE,    S     ,  Write a snapshot (PNG format)                        , snapshot                 ()); 
      glGribWindowIfKey (NONE,    Q     ,  Decrease field of view                               , shrink_fov               ());
      glGribWindowIfKey (NONE,    P     ,  Make earth flat/show orography                       , toggle_flat              ());
      glGribWindowIfKey (NONE,    6     ,  Increase size of current field                       , increaseRadius          ());
      glGribWindowIfKey (NONE,    EQUAL ,  Decrease size of current field                       , decreaseRadius          ());
      glGribWindowIfKey (NONE,    SPACE ,  Reset view                                           , reset_view               ());
      glGribWindowIfKey (NONE,    UP    ,  Move northwards                                      , rotate_north             ());
      glGribWindowIfKey (NONE,    DOWN  ,  Move southwards                                      , rotate_south             ());
      glGribWindowIfKey (NONE,    LEFT  ,  Move westwards                                       , rotate_west              ());
      glGribWindowIfKey (NONE,    RIGHT ,  Move eastwards                                       , rotate_east              ());

      glGribWindowIfKey (NONE,    F1    ,  Select field #1                                      , select_field           ( 0));
      glGribWindowIfKey (NONE,    F2    ,  Select field #2                                      , select_field           ( 1));
      glGribWindowIfKey (NONE,    F3    ,  Select field #3                                      , select_field           ( 2));
      glGribWindowIfKey (NONE,    F4    ,  Select field #4                                      , select_field           ( 3));
      glGribWindowIfKey (NONE,    F5    ,  Select field #5                                      , select_field           ( 4));
      glGribWindowIfKey (NONE,    F6    ,  Select field #6                                      , select_field           ( 5));
      glGribWindowIfKey (NONE,    F7    ,  Select field #7                                      , select_field           ( 6));
      glGribWindowIfKey (NONE,    F8    ,  Select field #8                                      , select_field           ( 7));
      glGribWindowIfKey (NONE,    F9    ,  Select field #9                                      , select_field           ( 8));
      glGribWindowIfKey (NONE,    F10   ,  Select field #10                                     , select_field           ( 9));
      glGribWindowIfKey (NONE,    F11   ,  Select field #11                                     , select_field           (10));
      glGribWindowIfKey (NONE,    F12   ,  Select field #12                                     , select_field           (11));

      glGribWindowIfKey (CONTROL, F1    ,  Show only field #1                                   , { hideAllFields (); select_field ( 0); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, F2    ,  Show only field #2                                   , { hideAllFields (); select_field ( 1); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, F3    ,  Show only field #3                                   , { hideAllFields (); select_field ( 2); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, F4    ,  Show only field #4                                   , { hideAllFields (); select_field ( 3); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, F5    ,  Show only field #5                                   , { hideAllFields (); select_field ( 4); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, F6    ,  Show only field #6                                   , { hideAllFields (); select_field ( 5); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, F7    ,  Show only field #7                                   , { hideAllFields (); select_field ( 6); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, F8    ,  Show only field #8                                   , { hideAllFields (); select_field ( 7); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, F9    ,  Show only field #9                                   , { hideAllFields (); select_field ( 8); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, F10   ,  Show only field #10                                  , { hideAllFields (); select_field ( 9); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, F11   ,  Show only field #11                                  , { hideAllFields (); select_field (10); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, F12   ,  Show only field #12                                  , { hideAllFields (); select_field (11); toggle_hide_field (); });
      glGribWindowIfKey (CONTROL, H     ,  Show all fields                                      , show_all_fields          ());
      glGribWindowIfKey (ALT,     H     ,  Show help                                            , showHelp                 ());

      glGribWindowIfKey (NONE,    H     ,  Show/hide selected field                             , toggle_hide_field        ());
      glGribWindowIfKey (NONE,    G     ,  Increase size of current field                       , scale_field_up           ());
      glGribWindowIfKey (CONTROL, G     ,  Decrease size of current field                       , scale_field_down         ());
      glGribWindowIfKey (NONE,    F     ,  Increase palette range                               , scale_palette_up         ());
      glGribWindowIfKey (CONTROL, F     ,  Decrease palette range                               , scale_palette_down       ());
      glGribWindowIfKey (NONE,    J     ,  Try next palette                                     , nextPalette             ());
      glGribWindowIfKey (NONE,    L     ,  Turn on/off the light                                , toggle_light             ());
      glGribWindowIfKey (CONTROL, L     ,  Make current window master window                    , toggleMaster             ());

      if (opts.fixLandscape.on)
      {
      glGribWindowIfKey (CONTROL, UP    ,                                                       , fixLandscape (+1,  0,  0,  0));
      glGribWindowIfKey (CONTROL, DOWN  ,                                                       , fixLandscape (-1,  0,  0,  0));
      glGribWindowIfKey (CONTROL, LEFT  ,                                                       , fixLandscape ( 0, -1,  0,  0));
      glGribWindowIfKey (CONTROL, RIGHT ,                                                       , fixLandscape ( 0, +1,  0,  0));
      glGribWindowIfKey (ALT,     UP    ,                                                       , fixLandscape ( 0,  0, +1,  0));
      glGribWindowIfKey (ALT,     DOWN  ,                                                       , fixLandscape ( 0,  0, -1,  0));
      glGribWindowIfKey (ALT,     LEFT  ,                                                       , fixLandscape ( 0,  0,  0, -1));
      glGribWindowIfKey (ALT,     RIGHT ,                                                       , fixLandscape ( 0,  0,  0, +1));
      }
      else
      {
      glGribWindowIfKey (CONTROL, UP    ,  Move light northwards                                , rotate_light_north       ());
      glGribWindowIfKey (CONTROL, DOWN  ,  Move light southwards                                , rotate_light_south       ());
      glGribWindowIfKey (CONTROL, LEFT  ,  Move light westwards                                 , rotate_light_west        ());
      glGribWindowIfKey (CONTROL, RIGHT ,  Move light eastwards                                 , rotate_light_east        ());
      }

      glGribWindowIfKey (CONTROL, C     ,  Clone current window                                 , duplicate                ());
      glGribWindowIfKey (ALT,     C     ,  Show/hide colorbar                                   , toggleColorBar           ());
      glGribWindowIfKey (CONTROL, P     ,  Try next projection                                  , nextProjection          ());
      glGribWindowIfKey (SHIFT,   P     ,  Try next transformation                              , toggle_transform_type    ());
      glGribWindowIfKey (CONTROL, S     ,  Save current palette                                 , save_current_palette     ());
      glGribWindowIfKey (ALT,     S     ,  Resample current field                               , resample_current_field   ());
      glGribWindowIfKey (NONE,    V     ,  Hide/show vector arrows                              , toggle_show_vector       ());
      glGribWindowIfKey (CONTROL, V     ,  Hide/show vector norm                                , toggle_show_norm         ());
      glGribWindowIfKey (NONE,    U     ,  Start shell                                          , startShell               ());


    }

}

void glGribWindow::toggle_wireframe () 
{ 
  glGribField * f = scene.getCurrentField ();

  if (f == nullptr)
    {
      scene.d.landscape.toggle_wireframe (); 
      return;
    }

  f->toggle_wireframe ();

}

void glGribWindow::fixLandscape (float dy, float dx, float sy, float sx)
{
  glGribOptionsLandscapePosition o = scene.d.landscape.getOptions ().lonlat.position;

  float dlat = o.lat2 - o.lat1;
  float dlon = o.lon2 - o.lon1;

  o.lat1 += dy * 0.01;
  o.lat2 += dy * 0.01;
  o.lon1 += dx * 0.01;
  o.lon2 += dx * 0.01;

  o.lat1 -= sy * 0.01;
  o.lat2 += sy * 0.01;
  o.lon1 -= sx * 0.01;
  o.lon2 += sx * 0.01;

  if (o.lon1 > 180.0f)
    o.lon1 -= 360.0f;
  if (o.lon2 > 180.0f)
    o.lon2 -= 360.0f;

  scene.d.landscape.setPositionOptions (o);
}

void glGribWindow::toggleColorBar ()
{
  scene.toggleColorBar ();
}

void glGribWindow::showHelp () 
{
  onkey (0, 0, 0, 0, true);
}

static glGribFieldVector * getVector (glGribScene & scene)
{
  glGribField * f = scene.getCurrentField ();

  if (f == nullptr) 
    return nullptr;

  glGribFieldVector * v = nullptr;

  try
    {
      v = dynamic_cast<glGribFieldVector*>(f);
    }
  catch (const std::bad_cast & e)
    {
      v = nullptr;
    }

  return v;
}

void glGribWindow::toggle_show_vector ()
{
  glGribFieldVector * v = getVector (scene);
  if (v)
    v->toggleShowVector ();
}


void glGribWindow::toggle_show_norm ()
{
  glGribFieldVector * v = getVector (scene);
  if (v)
    v->toggleShowNorm ();
}

void glGribWindow::resample_current_field ()
{
  glGribField * f = scene.getCurrentField ();

  if (f == nullptr) 
    return;

  glGribFieldVector * v = nullptr;

  try
    {
      v = dynamic_cast<glGribFieldVector*>(f);
    }
  catch (const std::bad_cast & e)
    {
      return;
    }

  if (v == nullptr)
    return;

  v->reSample (scene.d.view);

}

void glGribWindow::save_current_palette ()
{
  glGribField * f = scene.getCurrentField ();
  if (f == nullptr)
    return;
  f->saveOptions ();
}

void glGribWindow::remove_field (int rank)
{
  glGribField * f = nullptr;
  if ((rank < 0) || (rank > scene.fieldlist.size ()-1))
    return;
  if (scene.fieldlist[rank] != nullptr)
    delete scene.fieldlist[rank];
  scene.fieldlist[rank] = f;
}

void glGribWindow::loadField (const glGribOptionsField & opts, int rank)
{

  if ((rank < 0) || (rank > 11))
    return;

  makeCurrent ();

  glGribField * f = new glGribFieldScalar ();
  f->setup (&scene.ld, opts);

  if (rank > scene.fieldlist.size () - 1)
    scene.fieldlist.push_back (f);
  else
    scene.fieldlist[rank] = f;

}

void glGribWindow::toggle_transform_type ()
{
  scene.d.view.toggleTransformType ();
}

void glGribWindow::nextProjection ()
{
  scene.d.view.nextProjection ();
}

void glGribWindow::duplicate ()
{
  cloned = true;
}

void glGribWindow::rotate_light_north ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  y += 5.0f;
  scene.setLightPos (x, y);
}

void glGribWindow::rotate_light_south ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  y -= 5.0f;
  scene.setLightPos (x, y);
}

void glGribWindow::rotate_light_west  ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  x -= 5.0f;
  scene.setLightPos (x, y);
}

void glGribWindow::rotate_light_east  ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  x += 5.0f;
  scene.setLightPos (x, y);
}

void glGribWindow::toggle_light ()
{
  if (scene.hasLight ())
    scene.unsetLight ();
  else
    scene.setLight ();
}

void glGribWindow::nextPalette ()
{
  glGribField * f = scene.getCurrentField ();
  if (f != nullptr)
    f->setNextPalette ();
}

void glGribWindow::scale_palette_up ()
{
  glGribField * fld  = scene.getCurrentField ();
  if (fld == nullptr)
    return;
  fld->scalePaletteUp ();
}

void glGribWindow::scale_palette_down ()
{
  glGribField * fld  = scene.getCurrentField ();
  if (fld == nullptr)
    return;
  fld->scalePaletteDown ();
}

void glGribWindow::select_field (int ifield)
{
  scene.setCurrentFieldRank (ifield);
}

void glGribWindow::scale_field_down ()
{
  glGribField * f = scene.getCurrentField ();
  if (f != nullptr)
    {
      const glGribOptionsField & o = f->getOptions ();
      f->setScale (o.scale - 0.01);
    }
}

void glGribWindow::scale_field_up ()
{
  glGribField * f = scene.getCurrentField ();
  if (f != nullptr)
    {
      const glGribOptionsField & o = f->getOptions ();
      f->setScale (o.scale + 0.01);
    }
}

void glGribWindow::toggle_hide_field ()
{
  glGribField * fld = scene.getCurrentField ();
  if (fld == nullptr)
    return;
  if (fld->visible ())
    fld->hide ();
  else
    fld->show ();
}

void glGribWindow::hideAllFields ()
{
  for (auto f : scene.fieldlist)
    if (f != nullptr)
      f->hide ();
}

void glGribWindow::show_all_fields ()
{
  for (auto f : scene.fieldlist)
    if (f != nullptr)
      f->show ();
}

int glGribWindow::getLatlonFromCursor (float * lat, float * lon)
{
  double xpos, ypos;

  glfwGetCursorPos (window, &xpos, &ypos);
  ypos = opts.height - ypos;
  
  return scene.d.view.getLatlonFromScreenCoords (xpos, ypos, lat, lon);
}

void glGribWindow::snapshot (const std::string & format)
{
  unsigned char * rgb = new unsigned char[opts.width * opts.height * 4];

  // glReadPixels does not seem to work well with all combinations of width/height
  // when GL_RGB is used; GL_RGBA on the other hand seems to work well
  // So we get it in RGBA mode and throw away the alpha channel
  glReadPixels (0, 0, opts.width, opts.height, GL_RGBA, GL_UNSIGNED_BYTE, rgb);

  for (int i = 0; i < opts.width * opts.height; i++)
    for (int j = 0; j < 3; j++)
      rgb[3*i+j] = rgb[4*i+j];


  // %N  -> snapshot_cnt
  // %D  -> date

  const glGribOptionDate * date = scene.getDate ();
  std::string dstr = date ? date->asString () : "";
  for (int i = 0; i < dstr.size (); i++)
    if ((dstr[i] == ' ') || (dstr[i] == '/'))
      dstr[i] = ':';

  std::string fmt = format;

  for (int i = 0; ; i++)
    {
      size_t pos = fmt.find ("%D", i);
      if (pos == std::string::npos)
        break;
      i = pos + 1;
      fmt.replace (pos, 2, dstr);
    }

  
  std::vector<int> posN;
  for (int i = 0; ; i++)
    {
      size_t pos = fmt.find ("%N", i);
      if (pos == std::string::npos)
        break;
      posN.push_back (pos);
      i = pos + 1;
    }
  
  std::string filename;

  if (posN.size () > 0)
    {
      while (1)
        {
          struct stat st;
          char cnt[16];
      
          sprintf (cnt, "%4.4d", snapshot_cnt);
           
      
          filename = fmt;
      
          for (int i = posN.size () - 1; i >= 0; i--)
            filename.replace (posN[i], 2, std::string (cnt));
      
          if (stat (filename.c_str (), &st) < 0)
            break;
          else
            snapshot_cnt++;
        }
      snapshot_cnt++;
    }
  else
    {
      filename = fmt;
    }

  glGribWritePng (filename, opts.width, opts.height, rgb);

  delete [] rgb;
}

void glGribWindow::framebuffer (const std::string & format)
{
  if (opts.antialiasing.on)
    {
      unsigned int framebufferMMSA;
      unsigned int texturebufferMMSA;
      unsigned int renderbufferMMSA;
      unsigned int framebufferPOST;
      unsigned int texturebufferPOST;

      glGenFramebuffers (1, &framebufferMMSA);
      glBindFramebuffer (GL_FRAMEBUFFER, framebufferMMSA);

      glGenTextures (1, &texturebufferMMSA);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, texturebufferMMSA);
      glTexImage2DMultisample (GL_TEXTURE_2D_MULTISAMPLE, opts.antialiasing.samples, 
		               GL_RGB, opts.width, opts.height, GL_TRUE);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, 0);
      glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                              GL_TEXTURE_2D_MULTISAMPLE, texturebufferMMSA, 0);

      glGenRenderbuffers (1, &renderbufferMMSA);
      glBindRenderbuffer (GL_RENDERBUFFER, renderbufferMMSA);
      glRenderbufferStorageMultisample (GL_RENDERBUFFER, opts.antialiasing.samples, 
		                        GL_DEPTH24_STENCIL8, opts.width, opts.height);
      glBindRenderbuffer (GL_RENDERBUFFER, 0);
      glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                                 GL_RENDERBUFFER, renderbufferMMSA);

      glBindFramebuffer (GL_FRAMEBUFFER, 0);

      if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error (std::string ("Framebuffer is not complete"));

      glGenFramebuffers (1, &framebufferPOST);
      glBindFramebuffer (GL_FRAMEBUFFER, framebufferPOST);

      glGenTextures (1, &texturebufferPOST);
      glBindTexture (GL_TEXTURE_2D, texturebufferPOST);
      glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, opts.width, opts.height, 
                    0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                              GL_TEXTURE_2D, texturebufferPOST, 0);	

      if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error (std::string ("Framebuffer is not complete"));

      glBindFramebuffer (GL_FRAMEBUFFER, framebufferMMSA);
   
      scene.display ();

      glBindFramebuffer (GL_READ_FRAMEBUFFER, framebufferMMSA);
      glBindFramebuffer (GL_DRAW_FRAMEBUFFER, framebufferPOST);
      glBlitFramebuffer (0, 0, opts.width, opts.height, 0, 0, opts.width, opts.height, 
                         GL_COLOR_BUFFER_BIT, GL_NEAREST);

      glBindFramebuffer (GL_FRAMEBUFFER, framebufferPOST);
   
      snapshot (format);

      glDeleteTextures (1, &texturebufferPOST);
      glDeleteFramebuffers (1, &framebufferPOST);
      glDeleteRenderbuffers (1, &renderbufferMMSA);
      glDeleteTextures (1, &texturebufferMMSA);
      glDeleteFramebuffers (1, &framebufferMMSA);
   
      glBindFramebuffer (GL_FRAMEBUFFER, 0);
    }
  else
    {
      unsigned int framebuffer;
      unsigned int renderbuffer;
      unsigned int texturebuffer;
   
      glGenFramebuffers (1, &framebuffer);
      glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
     
      glGenTextures (1, &texturebuffer);
      glBindTexture (GL_TEXTURE_2D, texturebuffer);
      glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, opts.width, 
                    opts.height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
     
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                              GL_TEXTURE_2D, texturebuffer, 0);
     
      glGenRenderbuffers (1, &renderbuffer);
      glBindRenderbuffer (GL_RENDERBUFFER, renderbuffer);
     
      glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, 
                             opts.width, opts.height); 
      glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                                 GL_RENDERBUFFER, renderbuffer); 
   
      if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error (std::string ("Framebuffer is not complete"));
   
      scene.display ();
   
      snapshot (format);
   
      glDeleteRenderbuffers (1, &renderbuffer); 
      glDeleteTextures (1, &texturebuffer);
      glDeleteFramebuffers (1, &framebuffer);

      glBindFramebuffer (GL_FRAMEBUFFER, 0);
    }

}

void glGribWindow::displayCursorPosition (double xpos, double ypos)
{
  float lat, lon;
  if (getLatlonFromCursor (&lat, &lon))
    {
      std::string title_;
      const glGribField * field = scene.getCurrentField ();
      if (field)
        {
          int jglo = field->getGeometry ()->latlon2index (lat, lon);
	  if (jglo >= 0)
            {
              char tmp[128];
              std::vector<float> value = field->getValue (jglo);
              sprintf (tmp, "%6.2f %6.2f %8d", lat, lon, jglo);
              title_ = std::string (tmp);
	      for (auto v : value)
                {
                  sprintf (tmp, " %8.3g", v);
                  title_ = title_ + std::string (tmp);
                }
	    }
        }
      if (title_ == "")
        {
          char tmp[128];
          sprintf (tmp, "%6.2f %6.2f", lat, lon);
          title_ = std::string (tmp);
        }
      scene.setMessage (title_);
      glfwSetWindowTitle (window, title_.c_str ());
      return;
    }
  glfwSetWindowTitle (window, title.c_str ());
}

void glGribWindow::toggle_cursorpos_display ()
{
  if (cursorpos)
    glfwSetCursorPosCallback (window, nullptr);
  else
    glfwSetCursorPosCallback (window, cursorPositionCallback);
  cursorpos = ! cursorpos;
  scene.setMessage (std::string (""));
  glfwSetWindowTitle (window, title.c_str ());
}

void glGribWindow::onclick (int button, int action, int mods)
{
  enum
  {
    NONE    = 0,
    SHIFT   = GLFW_MOD_SHIFT,
    CONTROL = GLFW_MOD_CONTROL,
    ALT     = GLFW_MOD_ALT
  };

#define ifClick(mm, k, action) \
do { \
if ((button == GLFW_MOUSE_BUTTON_##k) && (mm == mods)) \
  {                                                    \
    action;                                            \
    return;                                            \
  }                                                    \
} while (0)

  if (action == GLFW_PRESS) 
    {
      ifClick (NONE,    LEFT, centerViewAtCursorPos ());
      ifClick (CONTROL, LEFT, centerLightAtCursorPos ());
      ifClick (ALT,     LEFT, debugTriangleNumber ());
    }
}

void glGribWindow::centerLightAtCursorPos ()
{
  float lat, lon;
  if (getLatlonFromCursor (&lat, &lon))
    scene.setLightPos (lon, lat);
}

void glGribWindow::centerViewAtCursorPos ()
{
  glGribOptionsView o = scene.d.view.getOptions ();
  if (getLatlonFromCursor (&o.lat, &o.lon))
    {
      scene.d.view.setOptions (o);
      float xpos, ypos;
      scene.d.view.getScreenCoordsFromLatlon (&xpos, &ypos, o.lat, o.lon);
      glfwSetCursorPos (window, xpos, ypos);
    }
}

void glGribWindow::debugTriangleNumber ()
{
  glGribField * f = scene.getCurrentField ();
  float lon, lat;
  if (getLatlonFromCursor (&lat, &lon) && (f != nullptr))
    {
      const_glgrib_geometry_ptr geometry = f->getGeometry ();
      std::cout << " getTriangle = " << geometry->getTriangle (lon, lat) << std::endl;
    }
}

void glGribWindow::scroll (double xoffset, double yoffset)
{

  makeCurrent ();

  glGribOptionsView o = scene.d.view.getOptions ();

  if (yoffset > 0)
    {
      if (o.fov < 1.0f)
        o.fov += 0.1f;
      else
        o.fov += 1.0f;
    }
  else 
    {
      if (o.fov < 1.0f)
        o.fov -= 0.1f;
      else
        o.fov -= 1.0f;
      if (o.fov <= 0.0f)
        o.fov = 0.1f;
    }
  scene.d.view.setOptions (o);
  scene.resize ();

}

void glGribWindow::renderFrame ()
{

  nframes++;

  makeCurrent ();

  scene.update ();

  if (Shell.started ())
    Shell.lock ();

  scene.display (); 

  glfwSwapBuffers (window);

  if (Shell.started ())
    Shell.unlock ();

}

void glGribWindow::run (glGribShell * shell)
{
  renderFrame ();
  glfwPollEvents ();
  
  if ((glfwGetKey (window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
   || (glfwWindowShouldClose (window) != 0) || (shell && shell->closed ()))
    closed = true;

}


void glGribWindow::resize (int w, int h)
{

  opts.width = w;
  opts.height = h;

  makeCurrent ();

  glViewport (0, 0, opts.width, opts.height);
  scene.setViewport (opts.width, opts.height);

}

void glGribWindow::setHints ()
{
  if (opts.antialiasing.on)
    glfwWindowHint (GLFW_SAMPLES, opts.antialiasing.samples);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, opts.version_major);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, opts.version_minor);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if (opts.debug.on)
    glfwWindowHint (GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

}

static int idcount = 0;

glGribWindow::glGribWindow ()
{
  id_ = idcount++;
}


glGribWindow::glGribWindow (const glGribOptions & opts)
{
  create (opts);
}

void glGribWindow::create (const glGribOptions & o)
{
  id_ = idcount++;
  opts = o.window;

  if (opts.title == "")
    title = std::string ("Window #") + std::to_string (id_);
  else
    title = opts.title;


  createGFLWwindow (nullptr);

  t0 = currentTime ();

  if (opts.info.on)
    {
#define PR(x) \
  printf (" %-32s = %s\n", #x, glGetString (x))
      PR (GL_VENDOR);
      PR (GL_RENDERER);
      PR (GL_VERSION);
      PR (GL_EXTENSIONS);
    }

  if ((opts.position.x != -1) && (opts.position.y != -1))
    glfwSetWindowPos (window, opts.position.x, opts.position.y);
}

void glGribWindow::createGFLWwindow (GLFWwindow * context)
{
  setHints ();
  
  window = glfwCreateWindow (opts.width, opts.height, title.c_str (), nullptr, context);
  glfwSetWindowUserPointer (window, this);

  if (window == nullptr)
    {
      fprintf (stderr, "Failed to open GLFW window. "
                       "If you have an Intel GPU, they are not 3.3 compatible. "
                       "Try the 2.1 version of the tutorials.\n");
      glfwTerminate ();
      return;
    }

  makeCurrent ();
  glInit ();
  
  glewExperimental = true; // Needed for core profile
  if (glewInit () != GLEW_OK) 
    {
      fprintf (stderr, "Failed to initialize GLEW\n");
      glfwTerminate ();
      return;
    }
  
  if (opts.debug.on)
   {
     GLint flags; 
     glGetIntegerv (GL_CONTEXT_FLAGS, &flags);
     if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
       {
         glEnable (GL_DEBUG_OUTPUT);
         glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS); 
         glDebugMessageCallback (debugCallback, this);
         glDebugMessageControl (GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
       }
   }

  glfwSetInputMode (window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetKeyCallback (window, keyCallback);
  glfwSetScrollCallback (window, scroll_callback);
  glfwSetMouseButtonCallback (window, mouseButtonCallback);
  glfwSetFramebufferSizeCallback (window, resize_callback);  


}

  
glGribWindow::~glGribWindow ()
{
  if (window)
    glfwDestroyWindow (window);
  if (opts.statistics.on)
    {
      double t1 = currentTime ();
      printf ("Window #%d rendered %f frames/sec\n", id_, nframes/(t1 - t0));
    }
}

glGribWindow * glGribWindow::clone ()
{
  glGribWindow * w = new glGribWindow ();

#define COPY(x) do { w->x = x; } while (0)
  COPY (opts);

  if (w->opts.title == "")
    w->title = std::string ("Window");
  else
    w->title = w->opts.title;

  w->title = w->title + " #" + std::to_string (w->id_);

  w->createGFLWwindow (window); // use already existing context

  COPY (scene);                 // copy the scene; invoke operator=

#undef COPY

  cloned = false;

  return w;
}

#define GLMESS(x) case GL_DEBUG_SOURCE_##x: return #x
static const char * debugSource (unsigned int source)
{
  switch (source)
    {
      GLMESS (API); GLMESS (WINDOW_SYSTEM); GLMESS (SHADER_COMPILER);
      GLMESS (THIRD_PARTY); GLMESS (APPLICATION); GLMESS (OTHER);
    }
  return "UNKNOWN";
}
#undef GLMESS

#define GLMESS(x) case GL_DEBUG_TYPE_##x: return #x
static const char * debugType (unsigned int type)
{
  switch (type)
    {
      GLMESS (ERROR); GLMESS (DEPRECATED_BEHAVIOR); GLMESS (UNDEFINED_BEHAVIOR);
      GLMESS (PORTABILITY); GLMESS (PERFORMANCE); GLMESS (MARKER);
      GLMESS (PUSH_GROUP); GLMESS (POP_GROUP); GLMESS (OTHER);
    } 
  return "UNKNOWN";
}
#undef GLMESS

#define GLMESS(x) case GL_DEBUG_SEVERITY_##x: return #x
static const char * debugSeverity (unsigned int severity)
{
  switch (severity)
    {
      GLMESS (HIGH); GLMESS (MEDIUM);
      GLMESS (LOW); GLMESS (NOTIFICATION);
    } 
  return "UNKNOWN";
}
#undef GLMESS

void glGribWindow::debug (unsigned int source, unsigned int type, GLuint id, 
		           unsigned int severity, int length, const char * message)
{
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204) 
    return; 
  printf ("%-20s | %-20s | %-30s | %10d | %s\n", debugSource (source), 
          debugSeverity (severity), debugType (type), id, message);
}

void glGribWindow::setOptions (const glGribOptionsWindow & o)
{
  if ((o.width != opts.width) || (o.height != opts.height))
    {
      glfwSetWindowSize(window, o.width, o.height);
      opts.width = o.width;
      opts.height = o.height;
    }
  if (o.title != opts.title)
    {
      opts.title = o.title;
      glfwSetWindowTitle (window, opts.title.c_str ());
    }
  if ((o.position.x != opts.position.x) || (o.position.y != opts.position.y))
    {
      opts.position = o.position;
      glfwSetWindowPos (window, opts.position.x, opts.position.y);
    }
}

