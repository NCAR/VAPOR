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
    string                  varname;
    OptionParser::Boolean_T help;
    OptionParser::Boolean_T quiet;
    OptionParser::Boolean_T debug;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"varname", 1, "", "Name of variable"}, {"help", 0, "", "Print this message and exit"}, {"quiet", 0, "", "Operate quitely"}, {"debug", 0, "", "Debug mode"}, {NULL}};

OptionParser::Option_T get_options[] = {{"varname", Wasp::CvtToCPPStr, &opt.varname, sizeof(opt.varname)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
                                        {"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
                                        {NULL}};

const char *ProgName;

void test1(PyEngine &pyE)
{
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

    int rc = pyE.Calculate(script, inputVarNames, inputVarDims, inputVarArrays, outputVarNames, outputVarDims, outputVarArrays);
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
    if (!s.empty()) { cout << "test1 : " << s << endl; }

    cout << "test1 : " << status << endl;
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

    if (argc != 1) {
        cerr << "Usage: " << ProgName << " [options] " << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    PyEngine pyE;
    int      rc = pyE.Initialize();
    if (rc < 0) return (-1);

    test1(pyE);

    return 0;
}
