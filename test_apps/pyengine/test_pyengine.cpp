#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cassert>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/MyPython.h>
#include <vapor/PyEngine.h>

using namespace std;

using namespace Wasp;
using namespace VAPoR;

size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}

struct {
    int                     memsize;
    int                     level;
    int                     lod;
    int                     nthreads;
    string                  ftype;
    OptionParser::Boolean_T help;
    OptionParser::Boolean_T quiet;
    OptionParser::Boolean_T debug;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"memsize", 1, "2000", "Cache size in MBs"},
                                         {"level", 1, "0", "Multiresution refinement level. Zero implies coarsest resolution"},
                                         {"lod", 1, "0", "Level of detail. Zero implies coarsest resolution"},
                                         {"nthreads", 1, "0",
                                          "Specify number of execution threads "
                                          "0 => use number of cores"},
                                         {"ftype", 1, "vdc", "data set type (vdc|wrf|cf|mpas)"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {"quiet", 0, "", "Operate quitely"},
                                         {"debug", 0, "", "Debug mode"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {
    {"memsize", Wasp::CvtToInt, &opt.memsize, sizeof(opt.memsize)},    {"level", Wasp::CvtToInt, &opt.level, sizeof(opt.level)},     {"lod", Wasp::CvtToInt, &opt.lod, sizeof(opt.lod)},
    {"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)}, {"ftype", Wasp::CvtToCPPStr, &opt.ftype, sizeof(opt.ftype)},  {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},      {"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)}, {NULL}};

const char *ProgName;

void test_calculate()
{
    int rc = PyEngine::Initialize();
    if (rc < 0) return;

    vector<size_t>         dims = {2, 4, 6};
    string                 script = "C = A + B";
    vector<string>         inputVarNames = {"A", "B"};
    vector<vector<size_t>> inputVarDims = {dims, dims};
    vector<string>         outputVarNames = {"C"};
    vector<vector<size_t>> outputVarDims = {dims};

    float *A = new float[vproduct(dims)];
    float *B = new float[vproduct(dims)];
    float *C = new float[vproduct(dims)];

    for (int i = 0; i < vproduct(dims); i++) {
        A[i] = 1;
        B[i] = 2;
    }

    vector<float *> inputVarArrays = {A, B};
    vector<float *> outputVarArrays = {C};

    rc = PyEngine::Calculate(script, inputVarNames, inputVarDims, inputVarArrays, outputVarNames, outputVarDims, outputVarArrays);
    if (rc < 0) return;

    string status = "SUCCESS";
    for (int i = 0; i < vproduct(dims); i++) {
        if (C[i] != A[i] + B[i]) {
            status = "FAILED!!!";
            break;
        }
    }

    delete[] A;
    delete[] B;
    delete[] C;

    string s = MyPython::Instance()->PyOut();
    if (!s.empty()) { cout << "test_calculate : " << s << endl; }

    cout << "test_calculate : " << status << endl;

    return;
}

void test_datamgr(vector<string> files)
{
    if (files.empty()) return;

    DataMgr datamgr(opt.ftype, opt.memsize, opt.nthreads);
    int     rc = datamgr.Initialize(files, vector<string>());
    if (rc < 0) exit(1);

    PyEngine pyEngine(&datamgr);
    if (pyEngine.Initialize() < 0) exit(1);

    vector<string> varNames = datamgr.GetDataVarNames();
    if (varNames.empty()) return;

    string inputVarName = varNames[0];
    string outputVarName = varNames[0] + "Copy";
    string script = outputVarName + " = " + inputVarName;

    DC::DataVar datavar;
    rc = datamgr.GetDataVarInfo(inputVarName, datavar);
    assert(rc >= 0);

    vector<string> inputVarNames = {inputVarName};
    vector<string> outputVarNames = {outputVarName};
    vector<string> outputMeshNames = {datavar.GetMeshName()};

    rc = pyEngine.AddFunction("myscript", script, inputVarNames, outputVarNames, outputMeshNames);
    if (rc < 0) exit(1);

    if (!opt.quiet) {
        cout << "Derived variable :\n";
        datamgr.GetDataVarInfo(outputVarName, datavar);
        cout << datavar;
    }

    Grid *g1 = datamgr.GetVariable(0, inputVarName, opt.level, opt.lod, true);
    if (!g1) exit(1);

    Grid *g2 = datamgr.GetVariable(0, outputVarName, opt.level, opt.lod, true);
    if (!g2) exit(1);

    Grid::ConstIterator itr1 = g1->cbegin();
    Grid::ConstIterator enditr1 = g1->cend();

    Grid::ConstIterator itr2 = g2->cbegin();
    Grid::ConstIterator enditr2 = g2->cend();

    string status = "SUCCESS";
    for (; itr1 != enditr1; ++itr1, ++itr2) {
        if (itr2 == enditr2) {
            status = "FAILED";
            break;
        }
        if (*itr1 != *itr2) {
            status = "FAILED";
            break;
        }
    }

    cout << "test_datamgr : " << status << endl;
}

int main(int argc, char **argv)
{
    OptionParser op;
    string       s;

    ProgName = Basename(argv[0]);

    MyBase::SetErrMsgFilePtr(stderr);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " [options] " << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    if (opt.debug) { MyBase::SetDiagMsgFilePtr(stderr); }

    vector<string> files;
    for (int i = 1; i < argc; i++) { files.push_back(argv[i]); }

    test_calculate();

    test_datamgr(files);

    return 0;
}
