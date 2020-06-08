#pragma once

#include "glGribShell.h"
#include "glGribOptionsShell.h"

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

class ShellRegular : public Shell
{
public:
  void setup (const OptionsShell &) override;
  void start (class WindowSet *) override;
  void run () override;
  char * optionGenerator (const char *, int);
  static ShellRegular & getInstance () 
  {
    return shellregular;
  }

  void lock () override { mutex.lock (); }
  void unlock () override { mutex.unlock (); }
  void wait () override { if (getWindowSet ()) thread.join (); }

private:
  ShellRegular ();
  ~ShellRegular () {}
  ShellRegular & operator= (const ShellRegular &) { return *this; }
  ShellRegular (const ShellRegular &) {}

  static ShellRegular shellregular;

  std::vector<std::string> tokenize (const std::string &);
  void runInt ();
  void runOff ();

  void process_help          (const std::vector<std::string> &, glGrib::Window *) override;
  void process_get           (const std::vector<std::string> &, glGrib::Window *) override;
  void process_window        (const std::vector<std::string> &, glGrib::Window *) override;
  void process_json          (const std::vector<std::string> &, glGrib::Window *) override;

  std::vector<std::string> getsetoptions;
  struct
  {
    int list_index, text_len;
  } og;
  std::thread thread;
  std::mutex mutex;
};

}
