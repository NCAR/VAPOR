//
//      $Id$
//

#ifndef _MYPYTHON_h_
#define _MYPYTHON_h_

#include <vapor/MyBase.h>

#ifdef WIN32
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#else
#include <Python.h>
#endif

namespace Wasp {

//
//! \class MYPYTHON
//! \brief A singleton class for initializing Python
//! \author John Clyne
//!
//! This singleton class should be used to initialize the Python
//! interpreter via Py_Initialize();
//!
//! Quoated from the Python 2.7.11 API documentation:
//!
//! Bugs and caveats: The destruction of modules and objects in modules
//! is done in random order; this may cause destructors (__del__() methods)
//! to fail when they depend on other objects (even functions) or modules.
//! Dynamically loaded extension modules loaded by Python are not
//! unloaded. Small amounts of memory allocated by the Python interpreter
//! may not be freed (if you find a leak, please report it). Memory tied
//! up in circular references between objects is not freed. Some memory
//! allocated by extension modules may not be freed. <b> Some extensions may
//! not work properly if their initialization routine is called more
//! than once; this can happen if an application calls Py_Initialize()
//! and Py_Finalize() more than once. </b>
//!
//! Usage: MyPython::Instance()->Initialize();
//
class COMMON_API MyPython : public Wasp::MyBase {

  public:
    static MyPython *Instance();

    //! Initialize the Python interpreter
    //!
    //! Checks value of VAPOR_PYHONHOME environment variable. If set,
    //! passes it's value to Py_SetPythonHome(). Otherwise uses GetAppsPath()
    //! to find the location of Python .so's and calls Py_SetPythonHome()
    //! with path returned by GetAppsPath(). Finally calls Py_Initialize()
    //
    void Initialize();

    //! Create a python function object from a script
    //!
    //! This method create a python function, named by \p funcName,
    //! from the Python script \p script. The script \p script must
    //! contain a python function definition for a function named by
    //! \p funcName. The function will be defined in the module
    //! named by \p moduleName. This module must already exist in
    //! the python environment.
    //!
    //! \param[in] moduleName Name of module in which to define
    //! the function.
    //!
    //! \param[in] name of function defined in the script \p script
    //!
    //! \param[in] script A string containing a valid python script
    //!
    //! \retval pFunc A new PyObject reference to the defined function,
    //! or NULL if any Python API calls fail
    //
    static PyObject *CreatePyFunc(
        string moduleName, string funcName, string script);

  private:
    static MyPython *m_instance;
    static bool m_isInitialized;
    static string m_pyHome;

    MyPython() {}                     // Don't implement
    MyPython(MyPython const &);       // Don't Implement
    void operator=(MyPython const &); // Don't implement
};
}; // namespace Wasp

#endif
