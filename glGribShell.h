#pragma once

#include "glGribWindow.h"
#include "glGribOptions.h"

#include <pthread.h>
#include <string>
#include <vector>
#include <map>
#include <list>

class glGribShell
{
public:
  glGribShell ();
  void setup (const glGribOptionsShell &);
  void execute (const std::string &, class glGribWindow *);
  int close = 0;
  bool closed () { return close; }
  void start (class glGribWindowSet *);
  void run ();
  void run_int ();
  void run_off ();
  int windowid = 0;
  void lock () { pthread_mutex_lock (&mutex); }
  void unlock () { pthread_mutex_unlock (&mutex); }
  void wait () { if (wset) pthread_join (thread, nullptr); }
  bool started () { return wset != nullptr; }
  char * option_generator (const char *, int);
  const glGribOptionsShell & getOptions () const { return opts; }
private:
  glGribOptionsShell opts;
  glGribWindowSet * wset = nullptr;
  pthread_t thread;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  std::vector<std::string> getsetoptions;
  struct
  {
    int list_index, text_len;
  } og;
};

extern glGribShell Shell;

