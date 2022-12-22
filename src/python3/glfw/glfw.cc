#include "glGrib/ShellInterpreter.h"
#include "glGrib/Resolve.h"
#include <Python.h>

namespace 
{

std::vector<PyMethodDef> glGribMethods;

class pymethoddef
{
public:
  pymethoddef (const char * name, PyCFunction method, const char * desc)
  {
    glGribMethods.push_back ({name, method, METH_VARARGS, desc});
  }
};

int buildArgList (std::vector<std::string> * argslist, PyObject * args)
{
  Py_ssize_t args_length = PyTuple_Size (args);

  for (Py_ssize_t i = 0; i < args_length; i++) 
    {
      PyObject * arg = PyTuple_GetItem (args, i);

      if (arg == NULL) 
        return 0;

      PyObject * str = PyObject_Str (arg);

      argslist->push_back (std::string ((char*)PyUnicode_AsUTF8 (str)));

      Py_DECREF (str);
    }

  return 1;
}

PyObject * createReturnList (const std::vector<std::string> & listStr)
{
  PyObject * list = PyList_New (listStr.size ());
  for (size_t i = 0; i < listStr.size (); i++)
    PyList_SetItem (list, i, Py_BuildValue ("s", listStr[i].c_str ()));
  return list;
}

PyObject * createReturnScal (const std::vector<std::string> & listStr)
{
  if (listStr.size () == 1)
    return Py_BuildValue ("s", listStr[0].c_str ());
  else if (listStr.size () > 1)
    return createReturnList (listStr);
  Py_RETURN_NONE;
}

typedef enum
{
  NONE=0,
  LIST=1,
  SCAL=2
} return_t;


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
      case NONE:
        break;
    }

  Py_RETURN_NONE;
}

#define createCommand(command, ret) \
PyObject * command (PyObject * self, PyObject * args)   \
{                                                       \
  return processCommand (#command, args, ret);          \
}                                                       \
static pymethoddef __##command (#command, command, "");

createCommand (start, NONE);
createCommand (set, NONE);
createCommand (sleep, NONE);
createCommand (stop, NONE);
createCommand (get, LIST);
createCommand (window, LIST);
createCommand (snapshot, NONE);
createCommand (clone, NONE);
createCommand (help, SCAL);
createCommand (json, SCAL);

}

static struct PyModuleDef glGribModule = 
{
  PyModuleDef_HEAD_INIT,
  "glGrib.glfw",
  "",
  -1,
  nullptr
};

PyMODINIT_FUNC
PyInit_glfw ()
{
  glGrib::glGribPrefix = GLGRIB_PREFIX;
  glGribMethods.push_back ({NULL, NULL, 0, NULL});
  glGribModule.m_methods = &glGribMethods[0];
  PyObject * m = PyModule_Create (&glGribModule);
  return m;
}



