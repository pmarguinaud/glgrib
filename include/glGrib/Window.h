#pragma once

#include "glGrib/OpenGL.h"

#include "glGrib/Render.h"
#include "glGrib/Scene.h"
#include "glGrib/View.h"
#include "glGrib/Shell.h"
#include "glGrib/Options.h"

#include <string>

#ifdef GLGRIB_USE_GLFW
namespace glGrib
{

class Shell;

class Window : public Render
{
public:

  static void getScreenSize (int *, int *);

  Window ();
  explicit Window (const Options &);
  virtual void setHints ();
  virtual ~Window ();
  void renderFrame (glGrib::Shell *);
  void run (class Shell * = nullptr) override;
  ContextGuard getContext () override;

  void toggleCursorposDisplay ();
  void toggleFlat ()
  {
    const auto & opts = scene.getLandscapeOptions ();
    scene.setLandscapeFlatOption (! opts.flat.on);
  }
  void resetView ()
  {
    OptionsView o0;
    const OptionsView & o = scene.getViewOptions ();
    o0.projection     = o.projection;
    o0.transformation = o.transformation;
    scene.setViewOptions (o0);
  }
  void toggleRotate      () 
  { 
    OptionsScene o = scene.getSceneOptions ();
    o.rotate_earth.on = ! o.rotate_earth.on; 
    scene.setSceneOptions (o);
  }
  void toggleRotateLight () 
  { 
    OptionsScene o = scene.getSceneOptions ();
    o.light.rotate.on = ! o.light.rotate.on; 
    scene.setSceneOptions (o);
  }
  void widenFov          () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.fov += 1.; 
    scene.setViewOptions (o);
  }
  void shrinkFov          () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.fov -= 1.; 
    scene.setViewOptions (o);
  }
  void increaseRadius     () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.distance += 0.1; 
    scene.setViewOptions (o);
  }
  void decreaseRadius     () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.distance -= 0.1; 
    scene.setViewOptions (o);
  }
  void rollLeft           () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.roll = float (o.roll) + 1.; 
    scene.setViewOptions (o);
  }
  void rollRight          () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.roll = float (o.roll) - 1.; 
    scene.setViewOptions (o);
  }
  void rotateUp           () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.pitch = float (o.pitch) + 1.; 
    scene.setViewOptions (o);
  }
  void rotateDown         ()
  {
    OptionsView o = scene.getViewOptions ();
    o.pitch = float (o.pitch) - 1.; 
    scene.setViewOptions (o);
  }
  void rotateRight        ()
  {
    OptionsView o = scene.getViewOptions ();
    o.yaw = float (o.yaw) + 1.; 
    scene.setViewOptions (o);
  }
  void rotateLeft         ()
  {
    OptionsView o = scene.getViewOptions ();
    o.yaw = float (o.yaw) - 1.; 
    scene.setViewOptions (o);
  }
  void rotateNorth        () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.lat = float (o.lat) + 5.; 
    scene.setViewOptions (o);
  }
  void rotateSouth        () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.lat = float (o.lat) - 5.; 
    scene.setViewOptions (o);
  }
  void rotateWest         () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.lon = float (o.lon) - 5.; 
    scene.setViewOptions (o);
  }
  void rotateEast         () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.lon = float (o.lon) + 5.; 
    scene.setViewOptions (o);
  }
  void toggleWireframe ();

  void scroll (double, double);
  void onclick (int, int, int);
  virtual void onkey (int, int, int, int, bool = false);
  void displayCursorPosition (double, double);
  int getLatLonFromCursor (float *, float *);
  void centerViewAtCursorPos ();
  void centerLightAtCursorPos ();
  void debugTriangleNumber ();

  void selectField (int);
  void selectGeoPoints (int);
  void scaleFieldUp ();
  void scaleFieldDown ();
  void toggleHideField ();
  void hideAllFields ();
  void showAllFields ();
  void toggleLight ();
  void rotateLightNorth ();
  void rotateLightSouth ();
  void rotateLightWest  ();
  void rotateLightEast  ();
  void duplicate          ();
  void create (const Options &);

  class Render * clone (bool = true) override;
  void shouldClose () override
  { 
    glfwSetWindowShouldClose (window, 1); 
  }
  
  void setOptions (const OptionsRender &) override;
  void nextProjection ();
  void toggleTransformType ();
  void saveCurrentPalette ();
  void resampleCurrentField ();
  void toggleShowVector ();
  void toggleShowNorm ();
  void toggleFullScreen ();
  void setFullScreen ();
  void showHelp ();

  void fixLandscape (float, float, float, float);

  void moveTo (int, int);
  void zoom (double, double);
  void zoomSchmidt (double, double);

private:
  bool cursorpos = false;

  GLFWwindow * window = nullptr;

  void showHelpItem (const char *, const char *, const char *, const char *);
  void createGFLWwindow (GLFWwindow * = nullptr);
  double t0;
  std::string title = "";
  struct
  {
    int x = 100, y = 100, w = 800, h = 800;
  } fullscreen;
};

}
#endif
