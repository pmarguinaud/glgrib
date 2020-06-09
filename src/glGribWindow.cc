#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glGribWindow.h"
#include "glGribPng.h"
#include "glGribFieldScalar.h"
#include "glGribFieldVector.h"
#include "glGribBuffer.h"

#include <iostream>
#include <stdexcept>


namespace
{

double currentTime ()
{
  struct timeval tv;
  struct timezone tz;
  gettimeofday (&tv, &tz);
  return static_cast<double> (tv.tv_sec) + static_cast<double> (tv.tv_usec) / 1e6;
}


void APIENTRY debugCallback (unsigned int source, unsigned int type, GLuint id, unsigned int severity, 
                             int length, const char * message, const void * data)
{
  glGrib::Window * gwindow = (glGrib::Window *)data;
  gwindow->debug (source, type, id, severity, length, message);
}


void cursorPositionCallback (GLFWwindow * window, double xpos, double ypos)
{
  glGrib::Window * gwindow = (glGrib::Window *)glfwGetWindowUserPointer (window);
  gwindow->displayCursorPosition (xpos, ypos);
}


void mouseButtonCallback (GLFWwindow * window, int button, int action, int mods)
{
  glGrib::Window * gwindow = (glGrib::Window *)glfwGetWindowUserPointer (window);
  gwindow->onclick (button, action, mods);
}


void scrollCallback (GLFWwindow * window, double xoffset, double yoffset)
{
  glGrib::Window * gwindow = (glGrib::Window *)glfwGetWindowUserPointer (window);
  gwindow->scroll (xoffset, yoffset);
}


void reSizeCallback (GLFWwindow * window, int width, int height)
{
  glGrib::Window * gwindow = (glGrib::Window *)glfwGetWindowUserPointer (window);
  gwindow->reSize (width, height);
}


void keyCallback (GLFWwindow * window, int key, int scancode, int action, int mods)
{
  glGrib::Window * gwindow = (glGrib::Window *)glfwGetWindowUserPointer (window);
  gwindow->onkey (key, scancode, action, mods);
}

}

void glGrib::Window::showHelpItem (const char * mm, const char * k, const char * desc, const char * action)
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

void glGrib::Window::getScreenSize (int * width, int * height)
{
  int _width, _height;
  if (width == nullptr)
    width = &_width;
  if (height == nullptr)
    height = &_height;
  GLFWmonitor * monitor = glfwGetPrimaryMonitor ();
  const GLFWvidmode * vmode = glfwGetVideoMode (monitor);
  *width = vmode->width;
  *height = vmode->height;
}

void glGrib::Window::setFullScreen ()
{
  if (! opts.fullscreen.on)
    {
      glfwSetWindowSize (window, fullscreen.w, fullscreen.h);
      moveTo (fullscreen.x, fullscreen.y);
    }
  else
    {
      glfwGetWindowPos (window, &fullscreen.x, &fullscreen.y);
      glfwGetWindowSize (window, &fullscreen.w, &fullscreen.h);

      int maxWidth, maxHeight;
      getScreenSize (&maxWidth, &maxHeight);

      glfwSetWindowSize (window, maxWidth, maxHeight);

      moveTo (0, 0);
   }
}


void glGrib::Window::toggleFullScreen ()
{
  opts.fullscreen.on = ! opts.fullscreen.on;
  setFullScreen ();
}

void glGrib::Window::onkey (int key, int scancode, int action, int mods, bool help)
{

#define glGribWindowIfKey(mm, k, desc, action) \
do { \
if (help)                                       \
  {                                             \
    showHelpItem (#mm, #k, #desc, #action);     \
  }                                             \
else if ((key == GLFW_KEY_##k) && (Window::mm == mods)) \
  {                                             \
    action;                                     \
    return;                                     \
  }                                             \
} while (0)

  if ((action == GLFW_PRESS || action == GLFW_REPEAT) || help)
    {

//    std::cout << glfwGetKeyName (key, scancode) << std::endl;
//    std::cout << glfwGetKeyName (key, glfwGetKeyScancode (key)) << std::endl;

      glGribWindowIfKey (CONTROL, F           ,  Toggle full screen mode, toggleFullScreen ());
      glGribWindowIfKey (NONE,    PAGE_UP     ,  One field forward,  next = true);
      glGribWindowIfKey (NONE,    PAGE_DOWN   ,  One field backward, prev = true);
      glGribWindowIfKey (NONE,    T     ,  Hide/show location & field value at cursor position  , toggleCursorposDisplay ());
      glGribWindowIfKey (NONE,    TAB   ,  Enable/disable earth rotation                        , toggleRotate            ());
      glGribWindowIfKey (CONTROL, TAB   ,  Enable/disable light rotation                        , toggleRotateLight      ());
      glGribWindowIfKey (NONE,    Y     ,  Display landscape or current field as wireframe      , toggleWireframe         ());
      glGribWindowIfKey (NONE,    D     ,  Use a framebuffer and generate a snapshot            , framebuffer              ());
      glGribWindowIfKey (NONE,    W     ,  Increase field of view                               , widen_fov                ());
      glGribWindowIfKey (NONE,    S     ,  Write a snapshot (PNG format)                        , snapshot                 ()); 
      glGribWindowIfKey (NONE,    Q     ,  Decrease field of view                               , shrinkFov               ());
      glGribWindowIfKey (NONE,    P     ,  Make earth flat/show orography                       , toggleFlat              ());
      glGribWindowIfKey (NONE,    6     ,  Increase size of current field                       , increaseRadius          ());
      glGribWindowIfKey (NONE,    EQUAL ,  Decrease size of current field                       , decreaseRadius          ());
      glGribWindowIfKey (NONE,    SPACE ,  Reset view                                           , resetView               ());
      glGribWindowIfKey (NONE,    UP    ,  Move northwards                                      , rotateNorth             ());
      glGribWindowIfKey (NONE,    DOWN  ,  Move southwards                                      , rotateSouth             ());
      glGribWindowIfKey (NONE,    LEFT  ,  Move westwards                                       , rotateWest              ());
      glGribWindowIfKey (NONE,    RIGHT ,  Move eastwards                                       , rotateEast              ());

      glGribWindowIfKey (NONE,    F1    ,  Select field #1                                      , selectField           ( 0));
      glGribWindowIfKey (NONE,    F2    ,  Select field #2                                      , selectField           ( 1));
      glGribWindowIfKey (NONE,    F3    ,  Select field #3                                      , selectField           ( 2));
      glGribWindowIfKey (NONE,    F4    ,  Select field #4                                      , selectField           ( 3));
      glGribWindowIfKey (NONE,    F5    ,  Select field #5                                      , selectField           ( 4));
      glGribWindowIfKey (NONE,    F6    ,  Select field #6                                      , selectField           ( 5));
      glGribWindowIfKey (NONE,    F7    ,  Select field #7                                      , selectField           ( 6));
      glGribWindowIfKey (NONE,    F8    ,  Select field #8                                      , selectField           ( 7));
      glGribWindowIfKey (NONE,    F9    ,  Select field #9                                      , selectField           ( 8));
      glGribWindowIfKey (NONE,    F10   ,  Select field #10                                     , selectField           ( 9));
      glGribWindowIfKey (NONE,    F11   ,  Select field #11                                     , selectField           (10));
      glGribWindowIfKey (NONE,    F12   ,  Select field #12                                     , selectField           (11));

      glGribWindowIfKey (CONTROL, F1    ,  Show only field #1                                   , { hideAllFields (); selectField ( 0); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F2    ,  Show only field #2                                   , { hideAllFields (); selectField ( 1); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F3    ,  Show only field #3                                   , { hideAllFields (); selectField ( 2); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F4    ,  Show only field #4                                   , { hideAllFields (); selectField ( 3); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F5    ,  Show only field #5                                   , { hideAllFields (); selectField ( 4); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F6    ,  Show only field #6                                   , { hideAllFields (); selectField ( 5); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F7    ,  Show only field #7                                   , { hideAllFields (); selectField ( 6); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F8    ,  Show only field #8                                   , { hideAllFields (); selectField ( 7); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F9    ,  Show only field #9                                   , { hideAllFields (); selectField ( 8); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F10   ,  Show only field #10                                  , { hideAllFields (); selectField ( 9); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F11   ,  Show only field #11                                  , { hideAllFields (); selectField (10); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F12   ,  Show only field #12                                  , { hideAllFields (); selectField (11); toggleHideField (); });
      glGribWindowIfKey (CONTROL, H     ,  Show all fields                                      , showAllFields          ());
      glGribWindowIfKey (ALT,     H     ,  Show help                                            , showHelp                 ());

      glGribWindowIfKey (NONE,    H     ,  Show/hide selected field                             , toggleHideField        ());
      glGribWindowIfKey (NONE,    G     ,  Increase size of current field                       , scaleFieldUp           ());
      glGribWindowIfKey (CONTROL, G     ,  Decrease size of current field                       , scaleFieldDown         ());
      glGribWindowIfKey (NONE,    L     ,  Turn on/off the light                                , toggleLight             ());
      glGribWindowIfKey (CONTROL, L     ,  Make current window master window                    , toggleMaster             ());

      if (opts.fixlandscape.on)
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
      glGribWindowIfKey (CONTROL, UP    ,  Move light northwards                                , rotateLightNorth       ());
      glGribWindowIfKey (CONTROL, DOWN  ,  Move light southwards                                , rotateLightSouth       ());
      glGribWindowIfKey (CONTROL, LEFT  ,  Move light westwards                                 , rotateLightWest        ());
      glGribWindowIfKey (CONTROL, RIGHT ,  Move light eastwards                                 , rotateLightEast        ());
      }

      glGribWindowIfKey (CONTROL, C     ,  Clone current window                                 , duplicate                ());
      glGribWindowIfKey (CONTROL, P     ,  Try next projection                                  , nextProjection          ());
      glGribWindowIfKey (SHIFT,   P     ,  Try next transformation                              , toggleTransformType    ());
      glGribWindowIfKey (CONTROL, S     ,  Save current palette                                 , saveCurrentPalette     ());
      glGribWindowIfKey (ALT,     S     ,  Resample current field                               , resampleCurrentField   ());
      glGribWindowIfKey (NONE,    V     ,  Hide/show vector arrows                              , toggleShowVector       ());
      glGribWindowIfKey (CONTROL, V     ,  Hide/show vector norm                                , toggleShowNorm         ());
      glGribWindowIfKey (NONE,    U     ,  Start shell                                          , startShell               ());


    }
#undef glGribWindowIfKey

}

void glGrib::Window::toggleWireframe () 
{ 
  glGrib::Field * f = scene.getCurrentField ();

  if (f == nullptr)
    {
      const auto & opts = scene.getLandscapeOptions ();
      scene.setLandscapeWireFrameOption (! opts.wireframe.on); 
      return;
    }

  f->toggleWireframe ();

}

void glGrib::Window::fixLandscape (float dy, float dx, float sy, float sx)
{
  glGrib::OptionsLandscapePosition o = scene.getLandscapeOptions ().lonlat.position;

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

  scene.setLandscapePositionOptions (o);
}

void glGrib::Window::showHelp () 
{
  onkey (0, 0, 0, 0, true);
}

namespace
{

glGrib::FieldVector * getVector (glGrib::Scene & scene)
{
  glGrib::Field * f = scene.getCurrentField ();

  if (f == nullptr) 
    return nullptr;

  glGrib::FieldVector * v = nullptr;

  try
    {
      v = dynamic_cast<glGrib::FieldVector*>(f);
    }
  catch (const std::bad_cast & e)
    {
      v = nullptr;
    }

  return v;
}

}

void glGrib::Window::toggleShowVector ()
{
  glGrib::FieldVector * v = getVector (scene);
  if (v)
    v->toggleShowVector ();
}


void glGrib::Window::toggleShowNorm ()
{
  glGrib::FieldVector * v = getVector (scene);
  if (v)
    v->toggleShowNorm ();
}

void glGrib::Window::resampleCurrentField ()
{
  glGrib::Field * f = scene.getCurrentField ();

  if (f == nullptr) 
    return;

  glGrib::FieldVector * v = nullptr;

  try
    {
      v = dynamic_cast<glGrib::FieldVector*>(f);
    }
  catch (const std::bad_cast & e)
    {
      return;
    }

  if (v == nullptr)
    return;

  v->reSample (scene.getView ());

}

void glGrib::Window::saveCurrentPalette ()
{
  glGrib::Field * f = scene.getCurrentField ();
  if (f == nullptr)
    return;
  f->saveOptions ();
}

void glGrib::Window::toggleTransformType ()
{
  scene.getView ().toggleTransformType ();
}

void glGrib::Window::nextProjection ()
{
  scene.getView ().nextProjection ();
}

void glGrib::Window::duplicate ()
{
  cloned = true;
}

void glGrib::Window::rotateLightNorth ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  y += 5.0f;
  scene.setLightPos (x, y);
}

void glGrib::Window::rotateLightSouth ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  y -= 5.0f;
  scene.setLightPos (x, y);
}

void glGrib::Window::rotateLightWest  ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  x -= 5.0f;
  scene.setLightPos (x, y);
}

void glGrib::Window::rotateLightEast  ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  x += 5.0f;
  scene.setLightPos (x, y);
}

void glGrib::Window::toggleLight ()
{
  if (scene.hasLight ())
    scene.unsetLight ();
  else
    scene.setLight ();
}

void glGrib::Window::selectField (int ifield)
{
  scene.setCurrentFieldRank (ifield);
}

void glGrib::Window::scaleFieldDown ()
{
  glGrib::Field * f = scene.getCurrentField ();
  if (f != nullptr)
    {
      const glGrib::OptionsField & o = f->getOptions ();
      f->setScale (o.scale - 0.01);
    }
}

void glGrib::Window::scaleFieldUp ()
{
  glGrib::Field * f = scene.getCurrentField ();
  if (f != nullptr)
    {
      const glGrib::OptionsField & o = f->getOptions ();
      f->setScale (o.scale + 0.01);
    }
}

void glGrib::Window::toggleHideField ()
{
  glGrib::Field * fld = scene.getCurrentField ();
  if (fld == nullptr)
    return;
  if (fld->visible ())
    fld->hide ();
  else
    fld->show ();
}

void glGrib::Window::hideAllFields ()
{
  scene.hideAllFields ();
}

void glGrib::Window::showAllFields ()
{
  scene.showAllFields ();
}

int glGrib::Window::getLatLonFromCursor (float * lat, float * lon)
{
  double xpos, ypos;

  glfwGetCursorPos (window, &xpos, &ypos);
  ypos = opts.height - ypos;
  
  return scene.getView ().getLatLonFromScreenCoords (xpos, ypos, lat, lon);
}

void glGrib::Window::snapshot (const std::string & format)
{
  BufferPtr<unsigned char> rgb (opts.width * opts.height * 4);

  // glReadPixels does not seem to work well with all combinations of width/height
  // when GL_RGB is used; GL_RGBA on the other hand seems to work well
  // So we get it in RGBA mode and throw away the alpha channel
  glReadPixels (0, 0, opts.width, opts.height, GL_RGBA, GL_UNSIGNED_BYTE, &rgb[0]);

  for (int i = 0; i < opts.width * opts.height; i++)
    for (int j = 0; j < 3; j++)
      rgb[3*i+j] = rgb[4*i+j];


  // %N  -> snapshot_cnt
  // %D  -> date

  const glGrib::OptionDate * date = scene.getDate ();
  std::string dstr = date ? date->asString () : "";
  for (size_t i = 0; i < dstr.size (); i++)
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

  glGrib::WritePng (filename, opts.width, opts.height, rgb);
}

void glGrib::Window::framebuffer (const std::string & format)
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
   
      scene.render ();

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
   
      scene.render ();
   
      snapshot (format);
   
      glDeleteRenderbuffers (1, &renderbuffer); 
      glDeleteTextures (1, &texturebuffer);
      glDeleteFramebuffers (1, &framebuffer);

      glBindFramebuffer (GL_FRAMEBUFFER, 0);
    }

}

void glGrib::Window::displayCursorPosition (double xpos, double ypos)
{
  float lat, lon;
  if (getLatLonFromCursor (&lat, &lon))
    {
      std::string title_;
      const glGrib::Field * field = scene.getCurrentField ();
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

void glGrib::Window::toggleCursorposDisplay ()
{
  if (cursorpos)
    glfwSetCursorPosCallback (window, nullptr);
  else
    glfwSetCursorPosCallback (window, cursorPositionCallback);
  cursorpos = ! cursorpos;
  scene.setMessage (std::string (""));
  glfwSetWindowTitle (window, title.c_str ());
}

void glGrib::Window::onclick (int button, int action, int mods)
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

#undef ifClick

}

void glGrib::Window::centerLightAtCursorPos ()
{
  float lat, lon;
  if (getLatLonFromCursor (&lat, &lon))
    scene.setLightPos (lon, lat);
}

void glGrib::Window::centerViewAtCursorPos ()
{
  glGrib::OptionsView o = scene.getViewOptions ();
  if (getLatLonFromCursor (&o.lat, &o.lon))
    {
      scene.setViewOptions (o);
      float xpos, ypos;
      scene.getView ().getScreenCoordsFromLatLon (&xpos, &ypos, o.lat, o.lon);
      glfwSetCursorPos (window, xpos, ypos);
    }
}

void glGrib::Window::debugTriangleNumber ()
{
  glGrib::Field * f = scene.getCurrentField ();
  float lon, lat;
  if (getLatLonFromCursor (&lat, &lon) && (f != nullptr))
    {
      glGrib::const_GeometryPtr geometry = f->getGeometry ();
      std::cout << " getTriangle = " << geometry->getTriangle (lon, lat) << std::endl;
    }
}

void glGrib::Window::zoom (double xoffset, double yoffset)
{
  makeCurrent ();

  glGrib::OptionsView o = scene.getViewOptions ();

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

  scene.setViewOptions (o);
}

void glGrib::Window::zoomSchmidt (double xoffset, double yoffset)
{
  makeCurrent ();

  glGrib::OptionsView o = scene.getViewOptions ();

  if (! o.zoom.on)
    {
      o.zoom.on = true;
      o.zoom.stretch = 1.0f;
    }

  o.zoom.lon = o.lon;
  o.zoom.lat = o.lat;

  if (yoffset > 0)
    o.zoom.stretch *= 1.01;
  else 
    o.zoom.stretch *= 0.99;

  scene.setViewOptions (o);
}

void glGrib::Window::scroll (double xoffset, double yoffset)
{

  enum
  {
    NONE    = 0,
    RIGHT_SHIFT   = GLFW_KEY_RIGHT_SHIFT,
    RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
    RIGHT_ALT     = GLFW_KEY_RIGHT_ALT,
    LEFT_SHIFT    = GLFW_KEY_LEFT_SHIFT,
    LEFT_CONTROL  = GLFW_KEY_LEFT_CONTROL,
    LEFT_ALT      = GLFW_KEY_LEFT_ALT 
  };

  glfwSetInputMode (window, GLFW_STICKY_KEYS, GL_FALSE);

#define ifScroll(mm, action) \
do { \
if ((mm == NONE) || (glfwGetKey (window, mm) == GLFW_PRESS)) \
  {                                                          \
    action;                                                  \
    return;                                                  \
  }                                                          \
} while (0)

  ifScroll (LEFT_CONTROL, zoomSchmidt (xoffset, yoffset));
  ifScroll (NONE,         zoom        (xoffset, yoffset));

#undef ifScroll

  glfwSetInputMode (window, GLFW_STICKY_KEYS, GL_TRUE);

}

void glGrib::Window::renderFrame (glGrib::Shell * shell)
{

  nframes++;

  makeCurrent ();

  scene.update ();

  if (shell && shell->started ())
    shell->lock ();

  scene.render (); 

  glfwSwapBuffers (window);

  if (shell && shell->started ())
    shell->unlock ();

}

void glGrib::Window::run (glGrib::Shell * shell)
{
  renderFrame (shell);
  glfwPollEvents ();
  
  if ((glfwGetKey (window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
   || (glfwWindowShouldClose (window) != 0) || (shell && shell->closed ()))
    close ();

}


void glGrib::Window::reSize (int w, int h)
{

  opts.width = w;
  opts.height = h;

  makeCurrent ();

  glViewport (0, 0, opts.width, opts.height);
  scene.setViewport (opts.width, opts.height);

}

void glGrib::Window::setHints ()
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

namespace
{
int idcount = 0;
}

glGrib::Window::Window ()
{
  id_ = idcount++;
}


glGrib::Window::Window (const glGrib::Options & _opts)
{
  create (_opts);
  scene.setup (_opts);
  reSize (opts.width, opts.height);
}

void glGrib::Window::create (const glGrib::Options & o)
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

void glGrib::Window::createGFLWwindow (GLFWwindow * context)
{
  setHints ();

  if (opts.fullscreen.on)
    getScreenSize (&opts.width, &opts.height);
  else if (opts.fullscreen.x.on)
    getScreenSize (&opts.width, nullptr);
  else if (opts.fullscreen.y.on)
    getScreenSize (nullptr, &opts.height);
  
  window = glfwCreateWindow (opts.width, opts.height, title.c_str (), nullptr, context);

  if (opts.fullscreen.on || opts.fullscreen.x.on || opts.fullscreen.y.on)
    moveTo (0, 0);

  glfwSetWindowUserPointer (window, this);

  if (window == nullptr)
    {
      fprintf (stderr, "Failed to open GLFW window.");
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
#define pp(x) \
     printf ("%-40s : %s\n", #x, glGetString (x));
     pp (GL_VENDOR);
     pp (GL_RENDERER);
     pp (GL_VERSION); 
     pp (GL_SHADING_LANGUAGE_VERSION);
#undef pp
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
  glfwSetScrollCallback (window, scrollCallback);
  glfwSetMouseButtonCallback (window, mouseButtonCallback);
  glfwSetFramebufferSizeCallback (window, reSizeCallback);  

}

  
glGrib::Window::~Window ()
{
  if (window)
    glfwDestroyWindow (window);
  if (opts.statistics.on)
    {
      double t1 = currentTime ();
      printf ("Window #%d rendered %f frames/sec\n", id_, nframes/(t1 - t0));
    }
}

glGrib::Window * glGrib::Window::clone ()
{
  glGrib::Window * w = new glGrib::Window ();

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

namespace
{


#define GLMESS(x) case GL_DEBUG_SOURCE_##x: return #x
const char * debugSource (unsigned int source)
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
const char * debugType (unsigned int type)
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
const char * debugSeverity (unsigned int severity)
{
  switch (severity)
    {
      GLMESS (HIGH); GLMESS (MEDIUM);
      GLMESS (LOW); GLMESS (NOTIFICATION);
    } 
  return "UNKNOWN";
}
#undef GLMESS

}

void glGrib::Window::debug (unsigned int source, unsigned int type, GLuint id, 
		           unsigned int severity, int length, const char * message)
{
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204) 
    return; 
  printf ("%-20s | %-20s | %-30s | %10d | %s\n", debugSource (source), 
          debugSeverity (severity), debugType (type), id, message);
}

void glGrib::Window::setOptions (const glGrib::OptionsWindow & o)
{
  if ((o.width != opts.width) || (o.height != opts.height))
    {
      glfwSetWindowSize (window, o.width, o.height);
      opts.width = o.width;
      opts.height = o.height;
    }
  if (o.title != opts.title)
    {
      opts.title = o.title;
      glfwSetWindowTitle (window, opts.title.c_str ());
    }
  if ((o.position.x != opts.position.x) || (o.position.y != opts.position.y))
    moveTo (o.position.x, o.position.y);
  if (opts.fullscreen.on != o.fullscreen.on)
    toggleFullScreen ();
}

void glGrib::Window::moveTo (int x, int y)
{
  opts.position.x = x; opts.position.y = y;
  glfwSetWindowPos (window, opts.position.x, opts.position.y);
}


