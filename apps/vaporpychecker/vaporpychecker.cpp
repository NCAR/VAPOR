#include <iostream>

#include <vapor/OptionParser.h>
#include <vapor/MyPython.h>
#include <vapor/CFuncs.h>

using namespace Wasp;

struct opt_t {
    OptionParser::Boolean_T quiet;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"quiet", 0, "", "Don't print anything, just exit with status"}, {"help", 0, "", "Print this message and exit"}, {NULL}};

OptionParser::Option_T get_options[] = {{"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)}, {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)}, {NULL}};

string ProgName;

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

int main(int argc, char **argv)
{
    OptionParser op;

    MyBase::SetErrMsgFilePtr(stderr);
    //
    // Parse command line arguments
    //
    ProgName = Basename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) { return (1); }

    if (op.ParseOptions(&argc, argv, get_options) < 0) { return (1); }

    if (argc != 1 || opt.help) {
        cerr << "Usage: " << ProgName << endl;
        op.PrintOptionHelp(stderr, 80, false);
        return (1);
    }

    // Ugh. Have to define a python object to enable capturing of
    // stderr to a string. Python API doesn't support a version of
    // PyErr_Print() that fetches the error to a C++ string. Give me
    // a break!
    //
    std::string stdErr = "import sys\n"
                         "class CatchErr:\n"
                         "   def __init__(self):\n"
                         "       self.value = 'Plot: '\n"
                         "   def write(self, txt):\n"
                         "       self.value += txt\n"
                         "catchErr = CatchErr()\n"
                         "sys.stderr = catchErr\n";

    // Use MyPython singleton class to initialize Python interpeter to
    // ensure it only gets initialized once.
    //
    MyPython::Instance()->Initialize();

    // Catch stderr from Python to a string.
    //
    int rc = PyRun_SimpleString(stdErr.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", pyErr().c_str());
        return (1);
    }

    cout << "System search path: ";
    std::string printSysPath = "import sys\n"
                               "print sys.path\n";

    rc = PyRun_SimpleString(printSysPath.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", pyErr().c_str());
        return (1);
    }

    cout << endl;
    cout << "Location of site module: ";
    std::string printSiteModulePath = "import sys\n"
                                      "import site\n"
                                      "print os.path.dirname(site.__file__)\n";

    rc = PyRun_SimpleString(printSiteModulePath.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", pyErr().c_str());
        return (1);
    }

    return (0);
}
