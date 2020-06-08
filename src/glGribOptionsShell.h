#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsShell : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Run command line);
    DESC_H (script, Script to execute);
    DESC (prompt.on, Start interactive session after batch script completion);
  }
  bool on = false; 
  std::string script;
  struct
  {
    bool on = false;
  } prompt;
};

};


