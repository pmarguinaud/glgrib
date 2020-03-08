#include "glGribShellInterpreter.h"
#include "glGribResolve.h"
#include <Python.h>

static 
int buildArgList (std::vector<std::string> * argslist, PyObject * args)
{
  Py_ssize_t args_length = PyTuple_Size (args);

  for (Py_ssize_t i = 0; i < args_length; i++) 
    {
      PyObject * arg = PyTuple_GetItem (args, i);

      if (arg == NULL) 
        return 0;

      PyObject * str = PyObject_Str (arg);

      argslist->push_back (std::string (PyString_AsString (str)));

      Py_DECREF (str);
    }

  return 1;
}

static 
PyObject * createReturnList (const std::vector<std::string> & listStr)
{
  PyObject * list = PyList_New (listStr.size ());
  for (int i = 0; i < listStr.size (); i++)
    PyList_SetItem (list, i, Py_BuildValue ("s", listStr[i].c_str ()));
  return list;
}

static 
PyObject * createReturnScal (const std::vector<std::string> & listStr)
{
  if (listStr.size () == 1)
    return Py_BuildValue ("s", listStr[0].c_str ());
  else if (listStr.size () > 1)
    return createReturnList (listStr);
  Py_RETURN_NONE;
}

static 
PyObject * start (PyObject * self, PyObject * _args)
{
  glGrib::ShellInterpreter & shell = glGrib::ShellInterpreter::getInstance ();

  std::vector<std::string> args;

  if (! buildArgList (&args, _args))
    return NULL;

  int argc = 1 + args.size ();
  const char * argv[argc];
  argv[0] = "";

  for (int i = 0; i < args.size (); i++)
    argv[1+i] = args[i].c_str ();

  shell.start (argc, argv);

  Py_RETURN_NONE;
}

typedef enum
{
  NONE=0,
  LIST=1,
  SCAL=2
} return_t;

static
PyObject * processCommand (const char * command, PyObject * _args, return_t ret)
{
  glGrib::ShellInterpreter & shell = glGrib::ShellInterpreter::getInstance ();

  std::vector<std::string> args;

  args.push_back (std::string (command));

  if (! buildArgList (&args, _args))
    return NULL;

  shell.execute (args);

  switch (ret)
    {
      case LIST:
        return createReturnList (shell.getList ());
      case SCAL:
        return createReturnScal (shell.getList ());
    }

  Py_RETURN_NONE;
}

#define createCommand(command, ret) \
static                                                  \
PyObject * command (PyObject * self, PyObject * args)   \
{                                                       \
  return processCommand (#command, args, ret);          \
}

createCommand (set, NONE);
createCommand (sleep, NONE);
createCommand (stop, NONE);
createCommand (get, LIST);
createCommand (window, LIST);
createCommand (snapshot, NONE);
createCommand (clone, NONE);
createCommand (help, SCAL);

static PyMethodDef glGribMethods[] = 
{
  {"start"   , start   , METH_VARARGS, "Start glGrib        "},
  {"stop"    , stop    , METH_VARARGS, "Stop glGrib         "},
  {"set"     , set     , METH_VARARGS, "Set glGrib options  "},
  {"sleep"   , sleep   , METH_VARARGS, "Make glGrib sleeping"},
  {"get"     , get     , METH_VARARGS, "Get glGrib options  "},
  {"window"  , window  , METH_VARARGS, "Select glGrib window"},
  {"snapshot", snapshot, METH_VARARGS, "Dump snapshot       "},
  {"clone"   , clone   , METH_VARARGS, "Clone current window"},
  {      NULL,     NULL,            0,                   NULL}
};

PyMODINIT_FUNC
initglGrib (void)
{
  glGrib::glGribPrefix = GLGRIB_PREFIX;
  Py_InitModule ("glGrib", glGribMethods);
}
