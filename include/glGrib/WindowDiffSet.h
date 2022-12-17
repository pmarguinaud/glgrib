#pragma once

#include "glGrib/Window.h"
#include "glGrib/Container.h"
#include "glGrib/WindowSet.h"

namespace glGrib
{

class WindowDiffSet : public WindowSet
{
public:
  explicit WindowDiffSet (const Options &);
  const std::string getNextExt () const;
  const std::string getPrevExt () const;
  void updateWindows () override;
private:
  Container * cont1 = nullptr;
  Container * cont2 = nullptr;
  std::string ext = "";
};

}
