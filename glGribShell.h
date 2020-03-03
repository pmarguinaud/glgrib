#pragma once

#include "glGribWindow.h"
#include "glGribOptions.h"

#include <pthread.h>
#include <string>
#include <vector>
#include <map>
#include <list>

namespace glGrib
{

class Shell
{
public:
  Shell ();
  void setup (const OptionsShell &);
  void execute (const std::string &, class Window *);
  int close = 0;
  bool closed () { return close; }
  void start (class WindowSet *);
  void run ();
  void runInt ();
  void runOff ();
  int windowid = 0;
  void lock () { pthread_mutex_lock (&mutex); }
  void unlock () { pthread_mutex_unlock (&mutex); }
  void wait () { if (wset) pthread_join (thread, nullptr); }
  bool started () { return wset != nullptr; }
  char * optionGenerator (const char *, int);
  const OptionsShell & getOptions () const { return opts; }
private:
  OptionsShell opts;
  WindowSet * wset = nullptr;
  pthread_t thread;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  std::vector<std::string> getsetoptions;
  struct
  {
    int list_index, text_len;
  } og;
};

extern Shell Shell0;


}
