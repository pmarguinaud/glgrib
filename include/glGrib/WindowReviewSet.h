#pragma once

#include "glGrib/Window.h"
#include "glGrib/Container.h"
#include "glGrib/WindowSet.h"

namespace glGrib
{

class WindowReviewSet : public WindowSet
{
public:
  explicit WindowReviewSet (const Options &);
  virtual ~WindowReviewSet () 
  {
  }
  const std::string getNextExt () const;
  const std::string getPrevExt () const;
  void updateWindows () override;
private:
  Container * cont = nullptr;
  std::string ext = "";
};

}
