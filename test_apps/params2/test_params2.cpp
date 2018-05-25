#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cassert>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/ViewpointParams.h>

using namespace Wasp;
using namespace VAPoR;

struct {
    OptionParser::Boolean_T help;
    OptionParser::Boolean_T quiet;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"help", 0, "", "Print this message and exit"}, {"quiet", 0, "", "Operate quitely"}, {NULL}};

OptionParser::Option_T get_options[] = {{"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)}, {"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)}, {NULL}};

const char *ProgName;

void test()
{
    ParamsBase::StateSave ssave;
    ParamsSeparator       parent(&ssave, "TOP");

    ViewpointParams vp1(&ssave);
    vp1.SetParent(&parent);

    ViewpointParams vp2(&ssave);
    // vp2.SetParent(&parent);

    if (vp1 == vp2) {
        cout << "SUCCESS: vp1 == vp2" << endl;
    } else {
        cout << "FAIL: vp1 == vp2" << endl;
    }

    vp1.SetWindowSize(1280, 1024);
    vp1.SetFOV(99.0);

    if (vp1 != vp2) {
        cout << "SUCCESS: vp1 != vp2" << endl;
    } else {
        cout << "FAIL: vp1 != vp2" << endl;
    }

    vp1 = vp2;
    if (vp1 == vp2) {
        cout << "SUCCESS: vp1 == vp2" << endl;
    } else {
        cout << "FAIL: vp1 == vp2" << endl;
    }
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
        cerr << "Usage: " << ProgName << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    MyBase::SetDiagMsgFilePtr(stderr);

    if (argc != 1) {
        cerr << "Usage: " << ProgName << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    test();

    cout << "Allocated note count after delete : " << XmlNode::GetAllocatedNodes().size() << endl;
}
