#pragma once

#include "glgrib_window.h"
#include "glgrib_options.h"

#include <pthread.h>
#include <string>
#include <vector>
#include <map>
#include <list>

class glgrib_shell
{
public:
  glgrib_shell ();
  void setup (const glgrib_options_shell &);
  void execute (const std::string &, class glgrib_window *);
  int close = 0;
  bool closed () { return close; }
  void start (class glgrib_window_set *);
  void run ();
  void run_int ();
  void run_off ();
  int windowid = 0;
  void lock () { pthread_mutex_lock (&mutex); }
  void unlock () { pthread_mutex_unlock (&mutex); }
  void wait () { if (wset) pthread_join (thread, NULL); }
  bool started () { return wset != NULL; }
  char * option_generator (const char *, int);
  const glgrib_options_shell & getOptions () const { return opts; }
private:
  glgrib_options_shell opts;
  glgrib_window_set * wset = NULL;
  pthread_t thread;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  std::vector<std::string> getsetoptions;
  struct
  {
    int list_index, text_len;
  } og;
};

extern glgrib_shell Shell;

