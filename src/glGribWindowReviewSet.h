#pragma once

#include "glGribWindow.h"
#include "glGribContainer.h"
#include "glGribWindowSet.h"

namespace glGrib
{

class WindowReviewSet : public WindowSet
{
public:
  WindowReviewSet (const Options &);
  const std::string getNextExt () const;
  const std::string getPrevExt () const;
  void updateWindows () override;
private:
  Container * cont = nullptr;
  std::string ext = "";
};

}
