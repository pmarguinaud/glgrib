#include "glGrib/WindowReviewSet.h"

#include <iostream>

namespace glGrib
{

WindowReviewSet::WindowReviewSet (const Options & o)
  : WindowSet (o, false)
{
  const auto & opts = getOptions ();
  Options opts_ = opts;

  if (opts.review.path->size () == 0)
    throw std::runtime_error (std::string ("Option --review.path is required"));


  cont = Container::create (opts.review.path, true);

  cont->buildIndex ();

  ext = getNextExt ();

  opts_.field[0].path.resize (1);
  opts_.field[0].path[0] = opts.review.path + "%" + ext;

  createWindow (opts_);
}

const std::string WindowReviewSet::getNextExt () const
{
  return cont->getNextExt (ext);
}

const std::string WindowReviewSet::getPrevExt () const
{
  return cont->getPrevExt (ext);
}

void WindowReviewSet::updateWindows ()
{
  Render * gwindow = getWindowById (0);
  if (gwindow == nullptr)
    return;

  std::string e;
  
  if (gwindow->getNext ())
    e = getNextExt ();
  
  if (gwindow->getPrev ())
    e = getPrevExt ();
  
  if (e == "")
    return;

  ext = e;
  
  auto fopts = getOptions ().field[0];
  
  fopts.path.resize (1);
  fopts.path[0] = cont->getFile () + "%" + ext;

  auto cg = gwindow->makeCurrent ();
  gwindow->getScene ().setFieldOptions (0, fopts);
  
}

}
