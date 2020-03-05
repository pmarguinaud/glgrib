#pragma once

#include "glGribWindow.h"
#include "glGribOptions.h"

#include <pthread.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <functional>

namespace glGrib
{

class ShellBase
{
public:
  virtual void setup (const OptionsShell &)  = 0;
  virtual void start (class WindowSet *)  = 0;
  virtual void run ()  = 0;
  virtual void execute (const std::vector<std::string> &, class Window *);
  bool closed () { return close; }
  void lock () { pthread_mutex_lock (&mutex); }
  void unlock () { pthread_mutex_unlock (&mutex); }
  const OptionsShell & getOptions () const { return opts; }
  void wait () { if (wset) pthread_join (thread, nullptr); }
  bool started () { return wset != nullptr; }

protected:
  int close = 0;
  int windowid = 0;

  virtual void do_get           (const std::vector<std::string> &, glGrib::Window *) = 0;
  virtual void do_close         (const std::vector<std::string> &, glGrib::Window *);
  virtual void do_snapshot      (const std::vector<std::string> &, glGrib::Window *);
  virtual void do_sleep         (const std::vector<std::string> &, glGrib::Window *);
  virtual void do_clone         (const std::vector<std::string> &, glGrib::Window *);
  virtual void do_set           (const std::vector<std::string> &, glGrib::Window *);
  virtual void do_window        (const std::vector<std::string> &, glGrib::Window *);
  virtual void do_window_select (const std::vector<std::string> &, glGrib::Window *);
  virtual void do_window_list   (const std::vector<std::string> &, glGrib::Window *) = 0;

  OptionsShell opts;
  WindowSet * wset = nullptr;
  pthread_t thread;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
};

class Shell : public ShellBase
{
public:
  Shell ();
  void setup (const OptionsShell &) override;
  void start (class WindowSet *) override;
  void run () override;
  char * optionGenerator (const char *, int);
private:
  std::vector<std::string> tokenize (const std::string &);
  void runInt ();
  void runOff ();

  void do_get           (const std::vector<std::string> &, glGrib::Window *) override;
  void do_window_list   (const std::vector<std::string> &, glGrib::Window *) override;


  std::vector<std::string> getsetoptions;
  struct
  {
    int list_index, text_len;
  } og;
};

extern Shell Shell0;


}
