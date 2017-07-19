#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cassert>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/TransferFunction.h>

using namespace Wasp;
using namespace VAPoR;

struct {
    string ifile;
    OptionParser::Boolean_T help;
    OptionParser::Boolean_T quiet;
    OptionParser::Boolean_T debug;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"ifile", 1, "", "Construct Xml tree from a file"},
    {"help", 0, "", "Print this message and exit"},
    {"quiet", 0, "", "Operate quitely"},
    {"debug", 0, "", "Debug mode"},
    {NULL}};

OptionParser::Option_T get_options[] = {
    {"ifile", Wasp::CvtToCPPStr, &opt.ifile, sizeof(opt.ifile)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
    {"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
    {NULL}};

const char *ProgName;

int main(int argc, char **argv) {

    OptionParser op;
    string s;

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

    if (opt.debug) {
        MyBase::SetDiagMsgFilePtr(stderr);
    }

    if (argc != 1) {
        cerr << "Usage: " << ProgName << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    ParamsBase::StateSave save;

    TransferFunction *tf = new TransferFunction(&save);

    tf->SaveToFile("/Users/clyne/foo.xml");

    tf->LoadFromFile("/Users/clyne/foo.xml");

    tf->SaveToFile("/Users/clyne/doo.xml");

    delete tf;

    cout << "Allocated note count after delete : " << XmlNode::GetAllocatedNodes().size() << endl;
}
