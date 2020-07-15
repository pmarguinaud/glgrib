#pragma once

#include "glGribShell.h"
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

class Render;

class ShellRegular : public Shell
{
public:
  void setup (const OptionsShell &) override;
  void start (class WindowSet *) override;
  void run () override;
  static ShellRegular & getInstance () 
  {
    if (shellregular == nullptr)
      shellregular = new ShellRegular ();
    return *shellregular;
  }

  void lock () override { mutex.lock (); }
  void unlock () override { mutex.unlock (); }
  void wait () override 
  { 
    if (! thread.joinable ()) 
      return; 
    if (getWindowSet ()) 
      thread.join (); 
  }

  bool isSynchronous ()
  {
    return synchronous;
  }

  // Should be private
  char * optionGenerator (const char *, int);
private:

  ShellRegular ();
  ~ShellRegular () {}
  ShellRegular & operator= (const ShellRegular &);
  ShellRegular (const ShellRegular &);

  static ShellRegular * shellregular;

  std::vector<std::string> tokenize (const std::string &);
  void runInt ();
  void runOff ();

  void process_help          (const std::vector<std::string> &, glGrib::Render *) override;
  void process_get           (const std::vector<std::string> &, glGrib::Render *) override;
  void process_window        (const std::vector<std::string> &, glGrib::Render *) override;
  void process_json          (const std::vector<std::string> &, glGrib::Render *) override;
  void process_list          (const std::vector<std::string> &, glGrib::Render *) override;
  void process_resolve       (const std::vector<std::string> &, glGrib::Render *) override;

  std::vector<std::string> getsetoptions;
  struct
  {
    int list_index, text_len;
  } og;
  std::thread thread;
  std::mutex mutex;
  bool synchronous = true;
};

}
