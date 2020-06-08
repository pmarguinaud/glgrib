#pragma once

#include "glGribWindow.h"
#include "glGribContainer.h"
#include "glGribWindowSet.h"

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
