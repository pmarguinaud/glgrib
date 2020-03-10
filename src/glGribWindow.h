#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glGribScene.h"
#include "glGribView.h"
#include "glGribShell.h"
#include "glGribOptions.h"

#include <string>

namespace glGrib
{

class Shell;

class Window
{
public:

  static void getScreenSize (int *, int *);

  enum
  {
    NONE    = 0,
    SHIFT   = GLFW_MOD_SHIFT,
    CONTROL = GLFW_MOD_CONTROL,
    ALT     = GLFW_MOD_ALT
  };

  Window ();
  Window (const Options &);
  virtual void setHints ();
  virtual ~Window ();
  virtual void renderFrame (glGrib::Shell *);
  virtual void run (class Shell * = nullptr);
  void makeCurrent () { glfwMakeContextCurrent (window); }
  void debug (unsigned int, unsigned int, GLuint, unsigned int, int, const char *);
  int snapshot_cnt = 0;
  Scene scene;
  bool cursorpos = false;
  GLFWwindow * window = nullptr;

  void toggleCursorposDisplay ();
  void framebuffer              (const std::string & = "snapshot_%N.png");
  void snapshot                 (const std::string & = "snapshot_%N.png");
  void toggleFlat ()
  {
    scene.d.landscape.toggleFlat ();
  }
  void resetView ()
  {
    OptionsView o0;
    const OptionsView & o = scene.d.view.getOptions ();
    o0.projection     = o.projection;
    o0.transformation = o.transformation;
    scene.d.view.setOptions (o0);
    scene.reSize ();
  }
  void toggleRotate       () 
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
  void widen_fov           () 
  { 
    OptionsView o = scene.d.view.getOptions ();
    o.fov += 1.; 
    scene.d.view.setOptions (o);
  }
  void shrinkFov          () 
  { 
    OptionsView o = scene.d.view.getOptions ();
    o.fov -= 1.; 
    scene.d.view.setOptions (o);
  }
  void increaseRadius     () 
  { 
    OptionsView o = scene.d.view.getOptions ();
    o.distance += 0.1; 
    scene.d.view.setOptions (o);
  }
  void decreaseRadius     () 
  { 
    OptionsView o = scene.d.view.getOptions ();
    o.distance -= 0.1; 
    scene.d.view.setOptions (o);
  }
  void rotateNorth        () 
  { 
    OptionsView o = scene.d.view.getOptions ();
    o.lat = o.lat + 5.; 
    scene.d.view.setOptions (o);
  }
  void rotateSouth        () 
  { 
    OptionsView o = scene.d.view.getOptions ();
    o.lat = o.lat - 5.; 
    scene.d.view.setOptions (o);
  }
  void rotateWest         () 
  { 
    OptionsView o = scene.d.view.getOptions ();
    o.lon = o.lon - 5.; 
    scene.d.view.setOptions (o);
  }
  void rotateEast         () 
  { 
    OptionsView o = scene.d.view.getOptions ();
    o.lon = o.lon + 5.; 
    scene.d.view.setOptions (o);
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
  void scalePaletteUp ();
  void scalePaletteDown ();
  void nextPalette ();
  void hideAllFields ();
  void showAllFields ();
  void toggleLight ();
  void rotateLightNorth ();
  void rotateLightSouth ();
  void rotateLightWest  ();
  void rotateLightEast  ();
  void duplicate          ();
  void create (const Options &);

  class Window * clone ();
  bool isClosed () { return closed; }
  bool isCloned () { return cloned; }
  void setCloned () { cloned = true; }
  void shouldClose () { glfwSetWindowShouldClose (window, 1); }
  
  int id () const { return id_; }

  void nextProjection ();
  void toggleTransformType ();
  void loadField (const OptionsField &, int = 0);
  void removeField (int);
  void saveCurrentPalette ();
  void resampleCurrentField ();
  void toggleShowVector ();
  void toggleShowNorm ();
  void toggleFullScreen ();
  void setFullScreen ();
  void showHelp ();

  bool isMaster () const { return master; }
  void setMaster () { master = true; }
  void unsetMaster () { master = false; }
  void toggleMaster () { master = ! master; }
  void toggleColorBar ();
  void setOptions (const OptionsWindow &);
  OptionsWindow getOptions () { return opts; }
  void startShell ()
  {
    start_shell = true;
  }
  bool getStartShell ()
  {
    bool _start_shell = start_shell;
    start_shell = false;
    return _start_shell;
  }

  void fixLandscape (float, float, float, float);

  const OptionsWindow & getOptions () const { return opts; }

  bool getNext ()
  {
    bool _next = next;
    next = false;
    return _next;
  }
  bool getPrev ()
  {
    bool _prev = prev;
    prev = false;
    return _prev;
  }
  void moveTo (int, int);
protected:
  void showHelpItem (const char *, const char *, const char *, const char *);
  void createGFLWwindow (GLFWwindow * = nullptr);
  bool closed = false;
  bool cloned = false;
  bool master = false;
  OptionsWindow opts;
private:
  bool next = false; // Next field
  bool prev = false; // Prev field
  bool start_shell = false; // Start shell
  int id_ = 0;
  double t0;
  int nframes = 0;
  std::string title = "";
  struct
  {
    int x = 100, y = 100, w = 800, h = 800;
  } fullscreen;
};


}
