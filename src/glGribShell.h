#pragma once

#include "glGribOptions.h"

#include <pthread.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <functional>

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
  void lock ();
  void unlock ();
  const OptionsShell & getOptions () const { return opts; }
  void wait () { if (wset) pthread_join (thread, nullptr); }
  bool started () { return wset != nullptr; }

protected:
  int close = 0;
  int windowid = 0;

  // Run command and store result in listStr
  void do_help          (const std::vector<std::string> &, glGrib::Window *);
  void do_get           (const std::vector<std::string> &, glGrib::Window *);
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
  virtual void process_close         (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_snapshot      (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_sleep         (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_clone         (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_set           (const std::vector<std::string> &, glGrib::Window *) {}
  virtual void process_window        (const std::vector<std::string> &, glGrib::Window *) {}

  OptionsShell opts;
  WindowSet * wset = nullptr;
  pthread_t thread;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  std::vector<std::string> listStr;

};

}
