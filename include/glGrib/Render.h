#pragma once

#include "glGrib/OpenGL.h"

#include "glGrib/Scene.h"
#include "glGrib/View.h"
#include "glGrib/Shell.h"
#include "glGrib/Options.h"

#include <string>
#include <utility>

namespace glGrib
{

class Shell;

class Render
{
public:

  class Context
  {
  public:
    virtual void lock () = 0;
    virtual void unlock () = 0;
  };

  class ContextGuard  
  {
  public:
    ContextGuard (Context * ctx = nullptr) : context (ctx)
    {
      if (context != nullptr)
        context->lock ();
    }
    ~ContextGuard ()
    {
      if (context != nullptr)
        context->unlock ();
    }
  private:
    Context * context;
  };

  Render () {}
  explicit Render (const Options &);
  virtual ~Render () {}
  virtual void run (class Shell * = nullptr) = 0;
  virtual ContextGuard getContext () = 0;

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

  void reSize (int, int);

  void framebuffer              (const std::string & = "snapshot_%N.png");
  void snapshot                 (const std::string & = "snapshot_%N.png");
  virtual class Render * clone (bool = true) = 0;
  virtual void shouldClose () = 0;
  bool isClosed () { return closed; }
  bool isCloned () { return cloned; }
  bool isCleared () { return cleared; }
  void setCloned () { cloned = true; }
  void setCleared () { cleared = true; }

  void clear ();
  
  int id () const { return id_; }

  bool isMaster () const { return master; }
  void setMaster () { master = true; }
  void unsetMaster () { master = false; }
  void toggleMaster () { master = ! master; }
  virtual void setOptions (const OptionsRender &) = 0;
  const OptionsRender getOptions () { return opts; }
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

  const OptionsRender & getOptions () const { return opts; }

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

  void debug (unsigned int, unsigned int, GLuint, unsigned int, int, const char *);

private:

  int snapshot_cnt = 0;
  Scene scene;

  bool closed = false;
  bool cloned = false;
  bool cleared = false;
  bool master = false;
  OptionsRender opts;
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
