//								 *
//		     Copyright (C)  2016			*
//     University Corporation for Atmospheric Research		*
//		     All Rights Reserved			*
//								*
//************************************************************************/
//
//	File:		MyPython.cpp
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		Thu Sep 29 13:29:44 MDT 2016
//
//	Description:
//
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <cstdlib>
#include <csetjmp>
#include <csignal>
#include <unistd.h>
#include <vapor/GetAppPath.h>
#include <vapor/MyPython.h>

using namespace Wasp;

MyPython *MyPython::m_instance = NULL;
bool      MyPython::m_isInitialized = false;
// string MyPython::m_pyHome;

MyPython *MyPython::Instance()
{
    if (!m_instance) { m_instance = new MyPython(); }
    return (m_instance);
}

int MyPython::Initialize()
{
    if (m_isInitialized) return (0);

    /*
    m_pyHome.clear();

    char *s = getenv("VAPOR_PYTHONHOME");
    if (s) m_pyHome = s;

    if (m_pyHome.empty()) {

        // Set pythonhome to the vapor installation (based on VAPOR_HOME)
        //
        vector<string> pths;

        // On windows use VAPOR_HOME/lib/python2.7; VAPOR_HOME works
        // on Linux and Mac
        //
#ifdef _WINDOWS
        pths.push_back("python27");
        m_pyHome = GetAppPath("VAPOR","", pths, true);
#else
        m_pyHome = GetAppPath("VAPOR","home", pths, true);
#endif
    }

    if (! m_pyHome.empty()) {
        struct STAT64 statbuf;
        if (STAT64((m_pyHome + "/lib/python2.7").c_str(), &statbuf) >= 0) {
            // N.B. the string passed to Py_SetPythonHome() must be
            // maintained in static storage :-(. However, the python
            // documentation promisses that it's value will not be changed
            //
            // It's also important to use forward slashes even on Windows.
            //
            Py_SetPythonHome((char *) m_pyHome.c_str());

            MyBase::SetDiagMsg(
                "Setting PYTHONHOME in the vaporgui app to %s\n", m_pyHome.c_str()
            );
        }
    }
    */

    // Prevent python from attempting to write a .pyc file on disk.
    //
    const char *env = "PYTHONDONTWRITEBYTECODE=1";
    char        env2[256];
    strcpy(env2, env);    // All this trouble is to eliminate a compiler warning
    putenv(env2);

    Py_Initialize();

    // Ugh. Have to define a python object to enable capturing of
    // stderr to a string. Python API doesn't support a version of
    // PyErr_Print() that fetches the error to a C++ string. Give me
    // a break!
    //
    std::string stdErr = "try:\n"
                         "	import sys, os\n"
                         "except: \n"
                         "	print >> sys.stderr, \'Failed to import sys\'\n"
                         "	raise\n"
                         "class CatchErr:\n"
                         "	def __init__(self):\n"
                         "		self.value = 'VAPOR_PY: '\n"
                         "	def write(self, txt):\n"
                         "		self.value += txt\n"
                         "catchErr = CatchErr()\n"
                         "sys.stderr = catchErr\n";

    // Catch stderr from Python to a string.
    //
    int rc = PyRun_SimpleString(stdErr.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", PyErr().c_str());
        return (-1);
    }

    // Import matplotlib
    //
    std::string importMPL = "try:\n"
                            "	import matplotlib\n"
                            "except: \n"
                            "	print >> sys.stderr, \'Failed to import matplotlib\'\n"
                            "	raise\n";
    rc = PyRun_SimpleString(importMPL.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", PyErr().c_str());
        return (-1);
    }

    // Add vapor modules to search path
    //
    std::vector<std::string> dummy;
    std::string              path = Wasp::GetAppPath("VAPOR", "share", dummy);
    path = "sys.path.append('" + path + "/python')\n";
    rc = PyRun_SimpleString(path.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", PyErr().c_str());
        return (-1);
    }

    m_isInitialized = true;

    return (0);
}

// Fetch an error message genereated by Python API.
//
string MyPython::PyErr()
{
    PyObject *pMain = PyImport_AddModule("__main__");

    PyObject *catcher = NULL;
    if (pMain && PyObject_HasAttrString(pMain, "catchErr")) { catcher = PyObject_GetAttrString(pMain, "catchErr"); }

    // If catcher is NULL the Python message will be written
    // to stderr. Otherwise it is writter to the catchErr object.
    //
    PyErr_Print();

    if (!catcher) { return ("Failed to initialize Python error catcher!!!"); }

    PyObject *output = PyObject_GetAttrString(catcher, "value");
    return (PyString_AsString(output));
}

PyObject *MyPython::CreatePyFunc(string moduleName, string funcName, string script)
{
    PyObject *pMain = PyImport_AddModule("__main__");
    if (!pMain) { return (NULL); }

    PyObject *pModule = PyImport_AddModule(moduleName.c_str());
    if (!pModule) { return (NULL); }

    // Get the dictionary object from my module so I can pass this
    // to PyRun_String
    //
    PyObject *pLocal = PyModule_GetDict(pModule);
    assert(pLocal != NULL);    // no fail

    PyObject *pGlobal = PyModule_GetDict(pMain);
    assert(pGlobal != NULL);    // no fail

    PyObject *pValue = PyRun_String(script.c_str(), Py_file_input, pGlobal, pLocal);
    if (!pValue) { return (NULL); }
    Py_DECREF(pValue);

    PyObject *pFunc = PyObject_GetAttrString(pModule, funcName.c_str());
    assert(pFunc != NULL);

    int rc = PyCallable_Check(pFunc);
    if (rc != 1) {    // Yes, this API call returns a 1 on success.

        Py_DECREF(pFunc);
        return (NULL);
    }

    return (pFunc);
}
