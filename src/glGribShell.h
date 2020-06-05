#pragma once

#include "glGribOptions.h"

#include <string>
#include <vector>
#include <map>
#include <list>
#include <functional>
#include <thread>
#include <mutex>

namespace glGrib
{

class Window;

class Shell
{
public:
  virtual void setup (const OptionsShell &)  = 0;
  virtual void start (class WindowSet *)  = 0;
  virtual void run ()  = 0;
  virtual void execute (const std::vector<std::string> &);
  bool closed () { return close; }
  void setClosed () { close = true; }
  void lock () 
  { 
    mutex.lock ();
  }
  void unlock () 
  { 
    mutex.unlock ();
  }
  const OptionsShell & getOptions () const { return opts; }
  void wait () { if (wset) thread.join (); }
  bool started () { return wset != nullptr; }

protected:
  int close = 0;
  int windowid = 0;

  // Run command and store result in listStr
  void do_help          (const std::vector<std::string> &, glGrib::Window *);
  void do_get           (const std::vector<std::string> &, glGrib::Window *);
  void do_json          (const std::vector<std::string> &, glGrib::Window *);
  void do_close         (const std::vector<std::string> &, glGrib::Window *);
  void do_snapshot      (const std::vector<std::string> &, glGrib::Window *);
  void do_sleep         (const std::vector<std::string> &, glGrib::Window *);
  void do_clone         (const std::vector<std::string> &, glGrib::Window *);
  void do_set           (const std::vector<std::string> &, glGrib::Window *);
  void do_window        (const std::vector<std::string> &, glGrib::Window *);
  void do_window_select (const std::vector<std::string> &, glGrib::Window *);
  void do_window_list   (const std::vector<std::string> &, glGrib::Window *);

  // Process command output
  virtual void process_help          (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_get           (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_json          (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_close         (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_snapshot      (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_sleep         (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_clone         (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_set           (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_window        (const std::vector<std::string> &, glGrib::Window *) {}

  OptionsShell opts;
  WindowSet * wset = nullptr;

  std::thread thread;
  std::mutex mutex;

  std::vector<std::string> listStr;

};

}
