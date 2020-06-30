#pragma once

#include "glGribOpenGL.h"

#include "glGribRender.h"
#include "glGribScene.h"
#include "glGribView.h"
#include "glGribShell.h"
#include "glGribOptions.h"

#include <string>

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
  void makeCurrent () override
  { 
#ifdef USE_GLFW
    glfwMakeContextCurrent (window); 
#endif
  }
  void debug (unsigned int, unsigned int, GLuint, unsigned int, int, const char *);

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
  void rotateNorth        () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.lat = o.lat + 5.; 
    scene.setViewOptions (o);
  }
  void rotateSouth        () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.lat = o.lat - 5.; 
    scene.setViewOptions (o);
  }
  void rotateWest         () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.lon = o.lon - 5.; 
    scene.setViewOptions (o);
  }
  void rotateEast         () 
  { 
    OptionsView o = scene.getViewOptions ();
    o.lon = o.lon + 5.; 
    scene.setViewOptions (o);
  }
  void toggleWireframe ();

  void reSize (int, int);
  void scroll (double, double);
  void onclick (int, int, int);
  virtual void onkey (int, int, int, int, bool = false);
  void displayCursorPosition (double, double);
  int getLatLonFromCursor (float *, float *);
  void centerViewAtCursorPos ();
  void centerLightAtCursorPos ();
  void debugTriangleNumber ();

  void selectField (int);
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

  class Render * clone () override;
  void shouldClose () override
  { 
#ifdef USE_GLFW
    glfwSetWindowShouldClose (window, 1); 
#endif
  }
  
  void setOptions (const OptionsWindow &) override;
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

#ifdef USE_GLFW
  GLFWwindow * window = nullptr;
#endif

  void showHelpItem (const char *, const char *, const char *, const char *);
#ifdef USE_GLFW
  void createGFLWwindow (GLFWwindow * = nullptr);
#endif
  double t0;
  std::string title = "";
  struct
  {
    int x = 100, y = 100, w = 800, h = 800;
  } fullscreen;
};


}
