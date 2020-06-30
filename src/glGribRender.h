#pragma once

#include "glGribOpenGL.h"

#include "glGribScene.h"
#include "glGribView.h"
#include "glGribShell.h"
#include "glGribOptions.h"

#include <string>

namespace glGrib
{

class Shell;

class Render
{
public:

  Render () {}
  explicit Render (const Options &);
  virtual ~Render () {}
  virtual void run (class Shell * = nullptr) = 0;
  virtual void makeCurrent () = 0;

  Scene & getScene ()
  {
    return scene;
  }

  void setup (const glGrib::Options & o)
  {
    scene.setup (o);
  }

  const Scene & getScene () const
  {
    return scene;
  }

  void framebuffer              (const std::string & = "snapshot_%N.png");
  void snapshot                 (const std::string & = "snapshot_%N.png");
  virtual class Render * clone () = 0;
  virtual void shouldClose () = 0;
  bool isClosed () { return closed; }
  bool isCloned () { return cloned; }
  void setCloned () { cloned = true; }
  
  int id () const { return id_; }

  bool isMaster () const { return master; }
  void setMaster () { master = true; }
  void unsetMaster () { master = false; }
  void toggleMaster () { master = ! master; }
  virtual void setOptions (const OptionsWindow &) = 0;
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

  void update ()
  {
    scene.update ();
  }

  void close ()
  {
    closed = true;
  }

  int & getSnapshotCnt ()
  {
    return snapshot_cnt;
  }
  
private:
  int snapshot_cnt = 0;
  Scene scene;

  bool closed = false;
  bool cloned = false;
  bool master = false;
  OptionsWindow opts;
  bool next = false; // Next field
  bool prev = false; // Prev field
  bool start_shell = false; // Start shell
  int id_ = 0;
  int nframes = 0;
  friend class Window;
  friend class WindowOffscreen;
  friend class Batch;
};


}
