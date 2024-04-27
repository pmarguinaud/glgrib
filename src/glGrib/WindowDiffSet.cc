#include "glGrib/WindowDiffSet.h"

#include <iostream>


namespace glGrib
{

namespace
{

void setDiffOptions (OptionsField & opts1, OptionsField & opts2, 
                     const std::string & path1, const std::string & path2)
{
  opts1.diff.on = true;
  opts1.path.clear ();
  opts1.path.push_back (path1);
  opts1.path.push_back (path2);
  opts1.palette.name = "cold_hot";
  opts1.user_pref.on = false;
  opts2.path.clear ();
  opts2.path.push_back (path2);
}

}

WindowDiffSet::WindowDiffSet (const Options & o)
  : WindowSet (o, false)
{
  Options opts1 = getOptions (), opts2 = getOptions ();

  if (getOptions ().diff.path.size () != 2)
    throw std::runtime_error (std::string ("Option --diff.path requires two arguments"));


  auto fixOpts = [] (Options * opts)
  {
    opts->diff.on = false;
    opts->diff.path.clear ();
    for (size_t i = 0; i < opts->field.size (); i++)
      opts->field[i].path.clear ();
  };

  fixOpts (&opts1);
  fixOpts (&opts2);

  int maxWidth = 2000;
#ifdef GLGRIB_USE_GLFW
  int maxHeight = 1000;
  Window::getScreenSize (&maxWidth, &maxHeight);
#endif

  opts1.render.position.x = 0;
  opts1.render.position.y = 0;
  opts1.render.width = maxWidth / 2;
  opts1.render.height = maxWidth / 2;

  opts2.render.position.x = opts1.render.position.x + opts1.render.width;
  opts2.render.position.y = 0;
  opts2.render.width = maxWidth / 2;
  opts2.render.height = maxWidth / 2;

  cont1 = Container::create (getOptions ().diff.path[0], true);
  cont2 = Container::create (getOptions ().diff.path[1], true);


  cont1->buildIndex ();
  cont2->buildIndex ();

  ext = getNextExt ();

  setDiffOptions (opts1.field[0], opts2.field[0], 
                  getOptions ().diff.path[0] + "%" + ext, 
                  getOptions ().diff.path[1] + "%" + ext);

  Render * gwindow1 = createWindow (opts1);
  Render * gwindow2 = gwindow1->clone ();

  gwindow2->setOptions (opts2.render);

  gwindow2->setup (opts2);

  gwindow1->setMaster ();

  insert (gwindow2);
  
}

const std::string WindowDiffSet::getNextExt () const
{
  std::string e = ext;
  while (1)
    {
      e = cont1->getNextExt (e);
      if (e == "") 
        break;
      if (cont2->hasExt (e))
        break;
      e = "";
    }
  return e;
}

const std::string WindowDiffSet::getPrevExt () const
{
  std::string e = ext;
  while (1)
    {
      e = cont1->getPrevExt (e);
      if (e == "") 
        break;
      if (cont2->hasExt (e))
        break;
      e = "";
    }
  return e;
}

void WindowDiffSet::updateWindows ()
{
  Render * gwindow1 = getWindowById (0);
  Render * gwindow2 = getWindowById (1);
  if ((gwindow1 == nullptr) || (gwindow2 == nullptr))
    return;

  std::string e;
  
  if (gwindow1->getNext () || gwindow2->getNext ())
    e = getNextExt ();
  
  if (gwindow1->getPrev () || gwindow2->getPrev ())
    e = getPrevExt ();
  
  
  if (e == "")
    return;

  ext = e;
  
  auto fopts1 = getOptions ().field[0];
  auto fopts2 = getOptions ().field[0];
  
  setDiffOptions (fopts1, fopts2, cont1->getFile () + "%" + ext, cont2->getFile () + "%" + ext);
  
  {
    // First window
    auto cg = gwindow1->makeCurrent ();
    gwindow1->getScene ().setFieldOptions (0, fopts1);
  }
  
  {
    // Second window
    auto cg = gwindow2->makeCurrent ();
    gwindow2->getScene ().setFieldOptions (0, fopts2);
  }
}

}
