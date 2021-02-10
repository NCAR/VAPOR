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
#include <algorithm>
#include <cstdlib>
#include <csignal>
#include <cstdlib>
#include <csetjmp>
#include <csignal>

#ifndef WIN32
    #include <unistd.h>
#endif

#include <vapor/ResourcePath.h>
#include <vapor/MyPython.h>
#include "vapor/VAssert.h"

using namespace Wasp;

#ifdef VAPOR3_0_0
namespace {

bool        pyIntFailed = false;
static void signal_handler(int sig)
{
    if (sig == SIGINT) {
        cerr << "Caught SIGINT\n";
        pyIntFailed = true;
    }
}

void init_signals(void)
{
    struct sigaction sigact;

    sigact.sa_handler = signal_handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, (struct sigaction *)NULL);
}
}    // namespace
#endif

MyPython *  MyPython::m_instance = NULL;
bool        MyPython::m_isInitialized = false;
std::string MyPython::m_pyHome = "";

MyPython *MyPython::Instance()
{
    if (!m_instance) { m_instance = new MyPython(); }
    return (m_instance);
}

int MyPython::Initialize()
{
    if (m_isInitialized) return (0);

    m_pyHome.clear();
    char *s = getenv("VAPOR3_HOME");
    if (s) m_pyHome = s;

    if (m_pyHome.empty()) {
        // On windows use VAPOR_HOME/lib/python2.7; VAPOR_HOME works
        // on Linux and Mac
        m_pyHome = GetPythonDir();
    }

    if (!m_pyHome.empty()) {
#ifdef WIN32
        std::string version = GetPythonVersion();
        version.erase(std::remove(version.begin(), version.end(), '.'), version.end());
        std::string pythonPath = m_pyHome + "\\Python" + version + ";";
        pythonPath = pythonPath + m_pyHome + "\\Python" + version + "\\Lib;";
        pythonPath = pythonPath + m_pyHome + "\\Python" + version + "\\Lib\\site-packages";
        _putenv_s("PYTHONPATH", pythonPath.c_str());
        std::wstring   widestr = std::wstring(m_pyHome.begin(), m_pyHome.end());
        const wchar_t *widecstr = widestr.c_str();
        Py_SetPythonHome((wchar_t *)widecstr);
        MyBase::SetDiagMsg("Setting PYTHONHOME in the vaporgui app to %s\n", m_pyHome.c_str());
#endif
    }

    // Prevent python from attempting to write a .pyc file on disk.
    //
    const char *env = "PYTHONDONTWRITEBYTECODE=1";
    char        env2[256];
    strcpy(env2, env);    // All this trouble is to eliminate a compiler warning
    putenv(env2);

#ifdef VAPOR3_0_0
    init_signals();
#endif

    // This is dependent on the environmental variable PYTHONHOME which is
    // set in vaporgui/main.cpp
    Py_Initialize();

#ifdef VAPOR3_0_0
    if (pyIntFailed) {
        SetErrMsg("Failed to initialize python : Py_Initialize() Failed");
        return (-1);
    }
#endif

    // Ugh. Have to define a python object to enable capturing of
    // stderr to a string. Python API doesn't support a version of
    // PyErr_Print() that fetches the error to a C++ string. Give me
    // a break!
    //
    std::string stdErr = "try:\n"
                         "	import sys\n"
                         "except: \n"
                         "	print('Failed to import sys')\n"
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

    std::string stdOut = "try:\n"
                         "	import sys\n"
                         "except: \n"
                         "	print('Failed to import sys')\n"
                         "	raise\n"
                         "class CatchOut:\n"
                         "	def __init__(self):\n"
                         "		self.value = ''\n"
                         "	def write(self, txt):\n"
                         "		self.value += txt\n"
                         "catchOut = CatchOut()\n"
                         "sys.stdout = catchOut\n";

    // Catch stdout from Python to a string.
    //
    rc = PyRun_SimpleString(stdOut.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", PyErr().c_str());
        return (-1);
    }

    // Import matplotlib
    //
    std::string importMPL = "try:\n"
                            "	import matplotlib\n"
                            "except: \n"
                            "	print('Failed to import matplotlib', file=sys.stderr)\n"
                            "	raise\n";
    rc = PyRun_SimpleString(importMPL.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", PyErr().c_str());
        return (-1);
    }

    // Add vapor modules to search path
    //
    std::string path = Wasp::GetSharePath("python");
    path = "sys.path.append('" + path + "')\n";
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
    char *    s = PyUnicode_AsUTF8(output);

    // Erase the string
    //
    PyObject *eStr = PyUnicode_FromString("");
    PyObject_SetAttrString(catcher, "value", eStr);
    Py_DECREF(eStr);

    return (s ? string(s) : string());
}

// Fetch an error message genereated by Python API.
//
string MyPython::PyOut()
{
    PyObject *pMain = PyImport_AddModule("__main__");

    PyObject *catcher = NULL;
    if (pMain && PyObject_HasAttrString(pMain, "catchOut")) { catcher = PyObject_GetAttrString(pMain, "catchOut"); }

    if (!catcher) { return (""); }

    PyObject *output = PyObject_GetAttrString(catcher, "value");
    char *    s = PyUnicode_AsUTF8(output);

    // Erase the string
    //
    PyObject *eStr = PyUnicode_FromString("");
    PyObject_SetAttrString(catcher, "value", eStr);
    Py_DECREF(eStr);

    return (s ? string(s) : string());
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
    VAssert(pLocal != NULL);    // no fail

    PyObject *pGlobal = PyModule_GetDict(pMain);
    VAssert(pGlobal != NULL);    // no fail

    PyObject *pValue = PyRun_String(script.c_str(), Py_file_input, pGlobal, pLocal);
    if (!pValue) { return (NULL); }
    Py_DECREF(pValue);

    PyObject *pFunc = PyObject_GetAttrString(pModule, funcName.c_str());
    VAssert(pFunc != NULL);

    int rc = PyCallable_Check(pFunc);
    if (rc != 1) {    // Yes, this API call returns a 1 on success.

        Py_DECREF(pFunc);
        return (NULL);
    }

    return (pFunc);
}
