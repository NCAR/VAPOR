#include <iostream>

#include <vapor/OptionParser.h>
#include <vapor/MyPython.h>
#include <vapor/CFuncs.h>
#include <vapor/FileUtils.h>

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
    return (PyUnicode_AsUTF8(output));
}

int main(int argc, char **argv)
{
    OptionParser op;

    MyBase::SetErrMsgFilePtr(stderr);
    //
    // Parse command line arguments
    //
    ProgName = FileUtils::LegacyBasename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) { return (1); }

    if (op.ParseOptions(&argc, argv, get_options) < 0) { return (1); }

    if (argc != 1 || opt.help) {
        cerr << "Usage: " << ProgName << endl;
        op.PrintOptionHelp(stderr, 80, false);
        return (1);
    }

    // Use MyPython singleton class to initialize Python interpeter to
    // ensure it only gets initialized once.
    //
    int rc = MyPython::Instance()->Initialize();
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to initialize python : %s", pyErr().c_str());
        printf("Failed to initialize python : %s\n", pyErr().c_str());
        return (1);
    }

    cout << "System search path: ";
    std::string printSysPath = "try:\n"
                               "	import sys\n"
                               "except: \n"
                               "	print >> sys.stderr, \'Failed to import sys\'\n"
                               "	raise\n"
                               "print(sys.path)\n";

    rc = PyRun_SimpleString(printSysPath.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", MyPython::Instance()->PyErr().c_str());
        return (1);
    }
    cout << endl;

    cout << "Location of site module: ";
    std::string printSiteModulePath = "try:\n"
                                      "	import site\n"
                                      "except: \n"
                                      "	print >> sys.stderr, \'Failed to import site\'\n"
                                      "	raise\n"
                                      "print(os.path.dirname(site.__file__))\n";

    rc = PyRun_SimpleString(printSiteModulePath.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", MyPython::Instance()->PyErr().c_str());
        return (1);
    }

    cout << endl;
    cout << "Location of matplotlib module: ";
    std::string printMatplotlibModulePath = "try:\n"
                                            "	import matplotlib\n"
                                            "except: \n"
                                            "	print >> sys.stderr, \'Failed to import matplotlib\'\n"
                                            "	raise\n"
                                            "print(os.path.dirname(matplotlib.__file__))\n";

    rc = PyRun_SimpleString(printMatplotlibModulePath.c_str());
    if (rc < 0) {
        MyBase::SetErrMsg("PyRun_SimpleString() : %s", MyPython::Instance()->PyErr().c_str());
        return (1);
    }

    return (0);
}
