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
#include <cstdlib>
#include <Python.h>
#include <vapor/GetAppPath.h>
#include <vapor/MyPython.h>

using namespace Wasp;

MyPython *MyPython::m_instance = NULL;
bool MyPython::m_isInitialized = false;
string MyPython::m_pyHome;

MyPython *MyPython::Instance() {
    if (!m_instance) {
        m_instance = new MyPython();
    }
    return (m_instance);
}

void MyPython::Initialize() {
    if (m_isInitialized)
        return;

    m_pyHome.clear();

    char *s = getenv("VAPOR_PYTHONHOME");
    if (s)
        m_pyHome = s;

    if (m_pyHome.empty()) {

        // Set pythonhome to the vapor installation (based on VAPOR_HOME)
        //
        vector<string> pths;

        // On windows use VAPOR_HOME/lib/python2.7; VAPOR_HOME works
        // on Linux and Mac
        //
#ifdef _WINDOWS
        pths.push_back("python2.7");
        m_pyHome = GetAppPath("VAPOR", "lib", pths, true);
#else
        m_pyHome = GetAppPath("VAPOR", "home", pths, true);
#endif
    }

    if (!m_pyHome.empty()) {

        // N.B. the string passed to Py_SetPythonHome() must be
        // maintained in static storage :-(. However, the python
        // documentation promisses that it's value will not be changed
        //
        // It's also important to use forward slashes even on Windows.
        //
        Py_SetPythonHome((char *)m_pyHome.c_str());

        MyBase::SetDiagMsg(
            "Setting PYTHONHOME in the vaporgui app to %s\n", m_pyHome.c_str());
    }

    Py_Initialize();

    m_isInitialized = true;
}

PyObject *MyPython::CreatePyFunc(
    string moduleName, string funcName, string script) {

    PyObject *pMain = PyImport_AddModule("__main__");
    if (!pMain) {
        return (NULL);
    }

    PyObject *pModule = PyImport_AddModule(moduleName.c_str());
    if (!pModule) {
        return (NULL);
    }

    // Get the dictionary object from my module so I can pass this
    // to PyRun_String
    //
    PyObject *pLocal = PyModule_GetDict(pModule);
    assert(pLocal != NULL); // no fail

    PyObject *pGlobal = PyModule_GetDict(pMain);
    assert(pGlobal != NULL); // no fail

    PyObject *pValue = PyRun_String(
        script.c_str(), Py_file_input, pGlobal, pLocal);
    if (!pValue) {
        return (NULL);
    }
    Py_DECREF(pValue);

    PyObject *pFunc = PyObject_GetAttrString(pModule, funcName.c_str());
    assert(pFunc != NULL);

    int rc = PyCallable_Check(pFunc);
    if (rc != 1) { // Yes, this API call returns a 1 on success.

        Py_DECREF(pFunc);
        return (NULL);
    }

    return (pFunc);
}
