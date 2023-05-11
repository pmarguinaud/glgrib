#ifdef GLGRIB_USE_GLFW
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include "glGrib/Window.h"
#include "glGrib/Png.h"
#include "glGrib/FieldScalar.h"
#include "glGrib/FieldVector.h"
#include "glGrib/Buffer.h"
#include "glGrib/OpenGL.h"

#include <iostream>
#include <stdexcept>

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

namespace glGrib
{

namespace
{

double currentTime ()
{
  struct timeval tv;
  struct timezone tz;
  gettimeofday (&tv, &tz);
  return static_cast<double> (tv.tv_sec) + static_cast<double> (tv.tv_usec) / 1e6;
}



void cursorPositionCallback (GLFWwindow * window, double xpos, double ypos)
{
  Window * gwindow = (Window *)glfwGetWindowUserPointer (window);
  gwindow->displayCursorPosition (xpos, ypos);
}


void mouseButtonCallback (GLFWwindow * window, int button, int action, int mods)
{
  Window * gwindow = (Window *)glfwGetWindowUserPointer (window);
  gwindow->onclick (button, action, mods);
}


void scrollCallback (GLFWwindow * window, double xoffset, double yoffset)
{
  Window * gwindow = (Window *)glfwGetWindowUserPointer (window);
  gwindow->scroll (xoffset, yoffset);
}


void reSizeCallback (GLFWwindow * window, int width, int height)
{
  Window * gwindow = (Window *)glfwGetWindowUserPointer (window);
  gwindow->reSize (width, height);
}


void keyCallback (GLFWwindow * window, int key, int scancode, int action, int mods)
{
  Window * gwindow = (Window *)glfwGetWindowUserPointer (window);
  gwindow->onkey (key, scancode, action, mods);
}

}

void Window::showHelpItem (const char * mm, const char * k, const char * desc, const char * action)
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

void Window::getScreenSize (int * width, int * height)
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

void Window::setFullScreen ()
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


void Window::toggleFullScreen ()
{
  opts.fullscreen.on = ! opts.fullscreen.on;
  setFullScreen ();
}

void Window::onkey (int key, int scancode, int action, int mods, bool help)
{

  enum
  {
    NONE    = 0,
    SHIFT   = GLFW_MOD_SHIFT,
    CONTROL = GLFW_MOD_CONTROL,
    ALT     = GLFW_MOD_ALT
  };

#define glGribWindowIfKey(mm, k, desc, action) \
do { \
if (help)                                       \
  {                                             \
    showHelpItem (#mm, #k, #desc, #action);     \
  }                                             \
else if ((key == GLFW_KEY_##k) && (mm == mods)) \
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
      glGribWindowIfKey (NONE,    T     ,  Hide/show location & field value at cursor position  , toggleCursorposDisplay  ());
      glGribWindowIfKey (NONE,    TAB   ,  Enable/disable earth rotation                        , toggleRotate            ());
      glGribWindowIfKey (CONTROL, TAB   ,  Enable/disable light rotation                        , toggleRotateLight       ());
      glGribWindowIfKey (NONE,    Y     ,  Display landscape or current field as wireframe      , toggleWireframe         ());
      glGribWindowIfKey (NONE,    D     ,  Use a framebuffer and generate a snapshot            , framebuffer             ());
      glGribWindowIfKey (NONE,    W     ,  Increase field of view                               , widenFov                ());
      glGribWindowIfKey (NONE,    S     ,  Write a snapshot (PNG format)                        , snapshot                ()); 
      glGribWindowIfKey (NONE,    Q     ,  Decrease field of view                               , shrinkFov               ());
      glGribWindowIfKey (NONE,    P     ,  Make earth flat/show orography                       , toggleFlat              ());
      glGribWindowIfKey (NONE,    6     ,  Increase size of current field                       , increaseRadius          ());
      glGribWindowIfKey (NONE,    EQUAL ,  Decrease size of current field                       , decreaseRadius          ());
      glGribWindowIfKey (NONE,    SPACE ,  Reset view                                           , resetView               ());

      glGribWindowIfKey (SHIFT,   LEFT  ,  Roll camera leftward                                 , rollLeft                ());
      glGribWindowIfKey (SHIFT,   RIGHT ,  Roll camera rightward                                , rollRight               ());

      glGribWindowIfKey (CONTROL, UP    ,  Rotate camera upward                                 , rotateUp                ());
      glGribWindowIfKey (CONTROL, DOWN  ,  Rotate camera downward                               , rotateDown              ());
      glGribWindowIfKey (CONTROL, LEFT  ,  Rotate camera leftward                               , rotateLeft              ());
      glGribWindowIfKey (CONTROL, RIGHT ,  Rotate camera rightward                              , rotateRight             ());

      glGribWindowIfKey (NONE,    UP    ,  Move northward                                       , rotateNorth             ());
      glGribWindowIfKey (NONE,    DOWN  ,  Move southward                                       , rotateSouth             ());
      glGribWindowIfKey (NONE,    LEFT  ,  Move westward                                        , rotateWest              ());
      glGribWindowIfKey (NONE,    RIGHT ,  Move eastward                                        , rotateEast              ());

      glGribWindowIfKey (NONE,    F1    ,  Select field #1                                      , selectField            (0));
      glGribWindowIfKey (NONE,    F2    ,  Select field #2                                      , selectField            (1));
      glGribWindowIfKey (NONE,    F3    ,  Select field #3                                      , selectField            (2));
      glGribWindowIfKey (NONE,    F4    ,  Select field #4                                      , selectField            (3));
      glGribWindowIfKey (NONE,    F5    ,  Select field #5                                      , selectField            (4));
      glGribWindowIfKey (NONE,    F6    ,  Select field #6                                      , selectField            (5));
      glGribWindowIfKey (NONE,    F7    ,  Select field #7                                      , selectField            (6));
      glGribWindowIfKey (NONE,    F8    ,  Select field #8                                      , selectField            (7));
      glGribWindowIfKey (NONE,    F9    ,  Select field #9                                      , selectField            (8));
      glGribWindowIfKey (NONE,    F10   ,  Select field #10                                     , selectField            (9));

      glGribWindowIfKey (CONTROL, F1    ,  Show only field #1                                   , { hideAllFields (); selectField (0); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F2    ,  Show only field #2                                   , { hideAllFields (); selectField (1); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F3    ,  Show only field #3                                   , { hideAllFields (); selectField (2); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F4    ,  Show only field #4                                   , { hideAllFields (); selectField (3); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F5    ,  Show only field #5                                   , { hideAllFields (); selectField (4); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F6    ,  Show only field #6                                   , { hideAllFields (); selectField (5); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F7    ,  Show only field #7                                   , { hideAllFields (); selectField (6); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F8    ,  Show only field #8                                   , { hideAllFields (); selectField (7); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F9    ,  Show only field #9                                   , { hideAllFields (); selectField (8); toggleHideField (); });
      glGribWindowIfKey (CONTROL, F10   ,  Show only field #10                                  , { hideAllFields (); selectField (9); toggleHideField (); });

      glGribWindowIfKey (SHIFT,   F1    ,  Select geopoints #1                                  , selectGeoPoints       (0));
      glGribWindowIfKey (SHIFT,   F2    ,  Select geopoints #2                                  , selectGeoPoints       (1));
      glGribWindowIfKey (SHIFT,   F3    ,  Select geopoints #3                                  , selectGeoPoints       (2));
      glGribWindowIfKey (SHIFT,   F4    ,  Select geopoints #4                                  , selectGeoPoints       (3));
      glGribWindowIfKey (SHIFT,   F5    ,  Select geopoints #5                                  , selectGeoPoints       (4));
      glGribWindowIfKey (SHIFT,   F6    ,  Select geopoints #6                                  , selectGeoPoints       (5));
      glGribWindowIfKey (SHIFT,   F7    ,  Select geopoints #7                                  , selectGeoPoints       (6));
      glGribWindowIfKey (SHIFT,   F8    ,  Select geopoints #8                                  , selectGeoPoints       (7));
      glGribWindowIfKey (SHIFT,   F9    ,  Select geopoints #9                                  , selectGeoPoints       (8));
      glGribWindowIfKey (SHIFT,   F10   ,  Select geopoints #10                                 , selectGeoPoints       (9));


      glGribWindowIfKey (CONTROL, H     ,  Show all fields                                      , showAllFields          ());
      glGribWindowIfKey (ALT,     H     ,  Show help                                            , showHelp               ());

      glGribWindowIfKey (NONE,    H     ,  Show/hide selected field                             , toggleHideField        ());
      glGribWindowIfKey (NONE,    G     ,  Increase size of current field                       , scaleFieldUp           ());
      glGribWindowIfKey (CONTROL, G     ,  Decrease size of current field                       , scaleFieldDown         ());
      glGribWindowIfKey (NONE,    L     ,  Turn on/off the light                                , toggleLight            ());
      glGribWindowIfKey (CONTROL, L     ,  Make current window master window                    , toggleMaster           ());

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

      glGribWindowIfKey (CONTROL, C     ,  Clone current window                                 , duplicate              ());
      glGribWindowIfKey (CONTROL, P     ,  Try next projection                                  , nextProjection         ());
      glGribWindowIfKey (SHIFT,   P     ,  Try next transformation                              , toggleTransformType    ());
      glGribWindowIfKey (CONTROL, S     ,  Save current palette                                 , saveCurrentPalette     ());
      glGribWindowIfKey (ALT,     S     ,  Resample current field                               , resampleCurrentField   ());
      glGribWindowIfKey (NONE,    V     ,  Hide/show vector arrows                              , toggleShowVector       ());
      glGribWindowIfKey (CONTROL, V     ,  Hide/show vector norm                                , toggleShowNorm         ());
      glGribWindowIfKey (NONE,    U     ,  Start shell                                          , startShell             ());


    }
#undef glGribWindowIfKey

}

void Window::toggleWireframe () 
{ 
  Field * f = scene.getCurrentField ();

  if (f == nullptr)
    {
      const auto & opts = scene.getLandscapeOptions ();
      scene.setLandscapeWireFrameOption (! opts.wireframe.on); 
      return;
    }

  f->toggleWireframe ();

}

void Window::fixLandscape (float dy, float dx, float sy, float sx)
{
  OptionsLandscapePosition o = scene.getLandscapeOptions ().lonlat.position;

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

void Window::showHelp () 
{
  onkey (0, 0, 0, 0, true);
}

namespace
{

FieldVector * getVector (Scene & scene)
{
  Field * f = scene.getCurrentField ();

  if (f == nullptr) 
    return nullptr;

  FieldVector * v = nullptr;

  try
    {
      v = dynamic_cast<FieldVector*>(f);
    }
  catch (const std::bad_cast & e)
    {
      v = nullptr;
    }

  return v;
}

}

void Window::toggleShowVector ()
{
  FieldVector * v = getVector (scene);
  if (v)
    v->toggleShowVector ();
}


void Window::toggleShowNorm ()
{
  FieldVector * v = getVector (scene);
  if (v)
    v->toggleShowNorm ();
}

void Window::resampleCurrentField ()
{
  Field * f = scene.getCurrentField ();

  if (f == nullptr) 
    return;

  FieldVector * v = nullptr;

  try
    {
      v = dynamic_cast<FieldVector*>(f);
    }
  catch (const std::bad_cast & e)
    {
      return;
    }

  if (v == nullptr)
    return;

  v->reSample (scene.getView ());

}

void Window::saveCurrentPalette ()
{
  Field * f = scene.getCurrentField ();
  if (f == nullptr)
    return;
  f->saveOptions ();
}

void Window::toggleTransformType ()
{
  scene.getView ().toggleTransformType ();
}

void Window::nextProjection ()
{
  scene.getView ().nextProjection ();
}

void Window::duplicate ()
{
  cloned = true;
}

void Window::rotateLightNorth ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  y += 5.0f;
  scene.setLightPos (x, y);
}

void Window::rotateLightSouth ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  y -= 5.0f;
  scene.setLightPos (x, y);
}

void Window::rotateLightWest  ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  x -= 5.0f;
  scene.setLightPos (x, y);
}

void Window::rotateLightEast  ()
{
  float x, y;
  scene.getLightPos (&x, &y);
  x += 5.0f;
  scene.setLightPos (x, y);
}

void Window::toggleLight ()
{
  if (scene.hasLight ())
    scene.unsetLight ();
  else
    scene.setLight ();
}

void Window::selectField (int ifield)
{
  scene.setCurrentFieldRank (ifield);
}

void Window::selectGeoPoints (int igeopoints)
{
  scene.setCurrentGeoPointsRank (igeopoints);
}

void Window::scaleFieldDown ()
{
  Field * f = scene.getCurrentField ();
  if (f != nullptr)
    {
      const OptionsField & o = f->getOptions ();
      f->setScale (o.scale - 0.01);
    }
}

void Window::scaleFieldUp ()
{
  Field * f = scene.getCurrentField ();
  if (f != nullptr)
    {
      const OptionsField & o = f->getOptions ();
      f->setScale (o.scale + 0.01);
    }
}

void Window::toggleHideField ()
{
  Field * fld = scene.getCurrentField ();
  if (fld == nullptr)
    return;
  if (fld->visible ())
    fld->hide ();
  else
    fld->show ();
}

void Window::hideAllFields ()
{
  scene.hideAllFields ();
}

void Window::showAllFields ()
{
  scene.showAllFields ();
}

int Window::getLatLonFromCursor (float * lat, float * lon)
{
  double xpos = 0, ypos = 0;

  glfwGetCursorPos (window, &xpos, &ypos);
  ypos = opts.height - ypos;
  
  return scene.getView ().getLatLonFromScreenCoords (xpos, ypos, lat, lon);
}

void Window::displayCursorPosition (double xpos, double ypos)
{
  float lat, lon;
  if (getLatLonFromCursor (&lat, &lon))
    {
      std::string title_;
      const Field * field = scene.getCurrentField ();
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

void Window::toggleCursorposDisplay ()
{
  if (cursorpos)
    glfwSetCursorPosCallback (window, nullptr);
  else
    glfwSetCursorPosCallback (window, cursorPositionCallback);
  cursorpos = ! cursorpos;
  scene.setMessage (std::string (""));
  glfwSetWindowTitle (window, title.c_str ());
}

void Window::onclick (int button, int action, int mods)
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

void Window::centerLightAtCursorPos ()
{
  float lat, lon;
  if (getLatLonFromCursor (&lat, &lon))
    scene.setLightPos (lon, lat);
}

void Window::centerViewAtCursorPos ()
{
  OptionsView o = scene.getViewOptions ();
  float lon, lat;
  if (getLatLonFromCursor (&lat, &lon))
    {
      o.lon = lon;
      o.lat = lat;
      scene.setViewOptions (o);
      float xpos, ypos;
      scene.getView ().getScreenCoordsFromLatLon (&xpos, &ypos, o.lat, o.lon);
      glfwSetCursorPos (window, xpos, ypos);
    }
}

void Window::debugTriangleNumber ()
{
  Field * f = scene.getCurrentField ();
  float lon, lat;
  if (getLatLonFromCursor (&lat, &lon) && (f != nullptr))
    {
      const_GeometryPtr geometry = f->getGeometry ();
      std::cout << " getTriangle = " << geometry->getTriangle (lon, lat) << std::endl;
    }
}

void Window::zoom (double xoffset, double yoffset)
{
  makeCurrent ();

  OptionsView o = scene.getViewOptions ();

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

void Window::zoomSchmidt (double xoffset, double yoffset)
{
  makeCurrent ();

  OptionsView o = scene.getViewOptions ();

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

void Window::scroll (double xoffset, double yoffset)
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

void Window::renderFrame (Shell * shell)
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

void Window::run (Shell * shell)
{
  renderFrame (shell);
  glfwPollEvents ();
  
  if ((glfwGetKey (window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
   || (glfwWindowShouldClose (window) != 0) || (shell && shell->closed ()))
    close ();
}

void Window::setHints ()
{
  if (opts.antialiasing.on)
    glfwWindowHint (GLFW_SAMPLES, opts.antialiasing.samples);

  auto version = getOpenGLVersion (opts.opengl.version);

  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, version.major);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, version.minor);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if (opts.debug.on)
    glfwWindowHint (GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
}

namespace
{
int idcount = 0;
}

Window::Window () 
{
  id_ = idcount++;
}


Window::Window (const Options & _opts) : Render::Render (_opts)
{
  create (_opts);
  scene.setup (_opts);
  reSize (opts.width, opts.height);
}

void Window::create (const Options & o)
{
  t0 = currentTime ();

  id_ = idcount++;
  opts = o.render;

  if (opts.title == "")
    title = std::string ("Window #") + std::to_string (id_);
  else
    title = opts.title;

  createGFLWwindow (nullptr);

  if ((opts.position.x != -1) && (opts.position.y != -1))
    glfwSetWindowPos (window, opts.position.x, opts.position.y);

  glSetupDebug (o.render);
}

void Window::createGFLWwindow (GLFWwindow * context)
{
  setHints ();

  if (opts.fullscreen.on)
    getScreenSize (&opts.width, &opts.height);
  else if (opts.fullscreen.x.on)
    getScreenSize (&opts.width, nullptr);
  else if (opts.fullscreen.y.on)
    getScreenSize (nullptr, &opts.height);
  

  {
    // GLFW sets its on handler when creating a new window; this may interfere with other X11 libraries such as PerlTk
    int (* handler)(Display *, XErrorEvent *) = XSetErrorHandler (nullptr);
    window = glfwCreateWindow (opts.width, opts.height, title.c_str (), nullptr, context);
    XSetErrorHandler (handler);
  }

  if (window == nullptr)
    {
      fprintf (stderr, "Failed to open GLFW window.");
      glfwTerminate ();
      return;
    }

  if (opts.fullscreen.on || opts.fullscreen.x.on || opts.fullscreen.y.on)
    moveTo (0, 0);

  glfwSetWindowUserPointer (window, this);

  makeCurrent ();
  glInit ();
  
  glewExperimental = true; // Needed for core profile
  if (glewInit () != GLEW_OK) 
    {
      fprintf (stderr, "Failed to initialize GLEW\n");
      glfwTerminate ();
      return;
    }
  
  glfwSetInputMode (window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetKeyCallback (window, keyCallback);
  glfwSetScrollCallback (window, scrollCallback);
  glfwSetMouseButtonCallback (window, mouseButtonCallback);
  glfwSetFramebufferSizeCallback (window, reSizeCallback);  

}

  
Window::~Window ()
{
  if (window)
    glfwDestroyWindow (window);
  if (opts.statistics.on)
    {
      double t1 = currentTime ();
      printf ("Window #%d rendered %f frames/sec\n", id_, nframes/(t1 - t0));
    }
}

Render * Window::clone (bool deep)
{
  Window * w = nullptr;

  w = new Window ();

#define COPY(x) do { w->x = x; } while (0)
  COPY (opts);

  if (w->opts.title == "")
    w->title = std::string ("Window");
  else
    w->title = w->opts.title;

  w->title = w->title + " #" + std::to_string (w->id_);

  w->createGFLWwindow (window); // use already existing context

  if (deep)
    COPY (scene);                 // copy the scene; invoke operator=

#undef COPY

  cloned = false;

  return w;
}

void Window::setOptions (const OptionsRender & o)
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

void Window::moveTo (int x, int y)
{
  opts.position.x = x; opts.position.y = y;
  glfwSetWindowPos (window, opts.position.x, opts.position.y);
}

}

#endif
