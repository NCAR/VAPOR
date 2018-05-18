#include <vapor/MyPython.h>

namespace VAPoR {
// Fetch an error message genereated by Python API.
//
string pyErr()
{
    PyObject *pMain = PyImport_AddModule("__main__");

    PyObject *catcher = NULL;
    if (pMain && PyObject_HasAttrString(pMain, "catchErr")) { catcher = PyObject_GetAttrString(pMain, "catchErr"); }

    // If catcher is NULL the Python message will be written
    // to stderr. Otherwise it is writter to the catchErr object.
    //
    PyErr_Print();

    if (!catcher) {
        cerr << "CATCHER NULL" << endl;
        return ("No Python error catcher");
    }

    PyObject *output = PyObject_GetAttrString(catcher, "value");
    return (PyString_AsString(output));
}

RENDER_API int Write_PNG(const char *file, int width, int height, unsigned char *buffer)
{
    PyObject *pName, *pModule, *pFunc, *pArgs, *pValue;

    int rc = Wasp::MyPython::Instance()->Initialize();
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to initialize python : %s", MyPython::Instance()->PyErr().c_str());
        return (-1);
    }

    std::string stdErr = "import sys\n"
                         "class CatchErr:\n"
                         "   def __init__(self):\n"
                         "       self.value = 'Plot: '\n"
                         "   def write(self, txt):\n"
                         "       self.value += txt\n"
                         "catchErr = CatchErr()\n"
                         "sys.stderr = catchErr\n";

    // Catch stderr from Python to a string.
    //
    if (PyRun_SimpleString(stdErr.c_str()) < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", pyErr().c_str());
        return (1);
    }

    cout << "System search path: ";
    std::string printSysPath = "import sys\n"
                               "print sys.path\n";

    int rc = PyRun_SimpleString(printSysPath.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", pyErr().c_str());
        return (1);
    }

    cout << endl;
    cout << "Location of site module: ";
    std::string printSiteModulePath = "import site\n"
                                      "print os.path.dirname(site.__file__)\n";

    rc = PyRun_SimpleString(printSiteModulePath.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", pyErr().c_str());
        return (1);
    }

    pName = PyString_FromString("imagewriter");
    pModule = PyImport_Import(pName);

    if (pModule == NULL) {
        MyBase::SetErrMsg("pModule (drawpng) NULL!!");
        std::cerr << "pModule (drawpng) NULL!!" << std::endl;
        PyErr_Print();
        return 1;
    }
    pFunc = PyObject_GetAttrString(pModule, "drawpng");
    if (pFunc && PyCallable_Check(pFunc)) {
        pArgs = PyTuple_New(4);

        // The 1st argument: output filename
        pValue = PyString_FromString(file);
        PyTuple_SetItem(pArgs, 0, pValue);

        // The 2nd argument: width
        pValue = PyInt_FromLong((long)width);
        PyTuple_SetItem(pArgs, 1, pValue);

        // The 3rd argument: height
        pValue = PyInt_FromLong((long)height);
        PyTuple_SetItem(pArgs, 2, pValue);

        // The 4th argument: RGB buffer
        long      nChars = width * height * 3;
        PyObject *pListOfChars = PyList_New(nChars);
        assert(pListOfChars);
        for (long i = 0; i < nChars; i++) {
            int rt = PyList_SetItem(pListOfChars, i, PyInt_FromLong((long)buffer[i]));
            assert(rt == 0);
        }
        PyTuple_SetItem(pArgs, 3, pListOfChars);

        // Call the python routine
        pValue = PyObject_CallObject(pFunc, pArgs);
        if (pValue == NULL) {
            MyBase::SetErrMsg("pFunc (drawpng) failed to execute!!");
            std::cerr << "pFunc (drawpng) failed to execute!!" << std::endl;
            PyErr_Print();
        }
    } else {
        MyBase::SetErrMsg("pFunc (drawpng) NULL!!");
        std::cerr << "pFunc (drawpng) NULL!!" << std::endl;
        PyErr_Print();
        return 1;
    }

    Py_XDECREF(pName);
    Py_XDECREF(pArgs);
    Py_XDECREF(pValue);
    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);

    return 0;
}
}    // namespace VAPoR
