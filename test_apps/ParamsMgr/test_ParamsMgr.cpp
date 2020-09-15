#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include "vapor/VAssert.h"

#include <vapor/MyBase.h>
#include <vapor/ParamsMgr.h>
#include <vapor/DataMgr.h>
#include <vapor/DataStatus.h>
#include <vapor/OptionParser.h>
#include <vapor/TwoDDataParams.h>
#include <vapor/CFuncs.h>
#include <vapor/FileUtils.h>

using namespace Wasp;
using namespace VAPoR;

struct {
    string                  ifile;
    OptionParser::Boolean_T help;
    OptionParser::Boolean_T quiet;
    OptionParser::Boolean_T debug;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"ifile", 1, "", "Construct Xml tree from a file"}, {"help", 0, "", "Print this message and exit"}, {"quiet", 0, "", "Operate quitely"}, {"debug", 0, "", "Debug mode"}, {NULL}};

OptionParser::Option_T get_options[] = {{"ifile", Wasp::CvtToCPPStr, &opt.ifile, sizeof(opt.ifile)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
                                        {"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
                                        {NULL}};

const char *ProgName;

int main(int argc, char **argv)
{
    OptionParser op;
    string       s;

    ProgName = FileUtils::LegacyBasename(argv[0]);

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

    if (opt.debug) { MyBase::SetDiagMsgFilePtr(stderr); }

    if (argc != 2) {
        cerr << "Usage: " << ProgName << " VDCMaster.nc [options] " << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    string vdcfile = argv[1];

    DataStatus dataStatus;
    string     dataSetName = "mydata";

    int rc = dataStatus.Open(vector<string>{vdcfile}, vector<string>(), dataSetName, "vdc");
    if (rc < 0) return (1);

    ParamsMgr *pm = new ParamsMgr();
    pm->SetSaveStateEnabled(false);

    string winName = "myWindow";

    pm->AddDataMgr(dataSetName, dataStatus.GetDataMgr(dataSetName));

    TwoDDataParams *twoDparams0 = (TwoDDataParams *)pm->CreateRenderParamsInstance(winName, dataSetName, TwoDDataParams::GetClassType(), "twoD0");

    twoDparams0->SetVariableName("U10");

    TwoDDataParams *twoDparams1 = (TwoDDataParams *)pm->CreateRenderParamsInstance(winName, dataSetName, TwoDDataParams::GetClassType(), "twoD1");

    twoDparams1->SetVariableName("V10");

    pm->SetSaveStateEnabled(true);
    pm->UndoRedoClear();

    pm->SaveToFile("file.xml");
    twoDparams1->SetVariableName("U1");
    twoDparams1->SetCompressionLevel(1);
    pm->RemoveVisualizer(winName);

    pm->Undo();
    pm->Undo();
    pm->Undo();

    pm->Redo();
    pm->Redo();
    pm->Redo();
    pm->Undo();
    pm->Undo();
    pm->Undo();

    pm->SaveToFile("fileUndo.xml");

    cout << "Allocated XmlNode count before delete " << XmlNode::GetAllocatedNodes().size() << endl;

//#define DEBUG
#ifdef DEBUG
    const vector<XmlNode *> &nodes = XmlNode::GetAllocatedNodes();
    for (int i = 0; i < nodes.size(); i++) { cout << "   " << nodes[i]->GetTag() << " " << nodes[i] << endl; }
#endif

    delete pm;

    cout << "Allocated XmlNode count after delete " << XmlNode::GetAllocatedNodes().size() << endl;

#ifdef DEBUG

    for (int i = 0; i < nodes.size(); i++) { cout << "   " << nodes[i]->GetTag() << " " << nodes[i] << endl; }
#endif
}
