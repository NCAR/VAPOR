#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cassert>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/TwoDImageParams.h>
#include <vapor/TwoDDataParams.h>
#include <vapor/ParamsStateMgr.h>
#include <vapor/TransferFunction.h>
#include <vapor/DataMgrV3_0.h>

using namespace Wasp;
using namespace VAPoR;


struct {
	string ifile;
	OptionParser::Boolean_T	help;
	OptionParser::Boolean_T	quiet;
	OptionParser::Boolean_T	debug;
} opt;

OptionParser::OptDescRec_T	set_opts[] = {
	{"ifile",	1,	"",	"Construct Xml tree from a file"},
	{"help",	0,	"",	"Print this message and exit"},
	{"quiet",	0,	"",	"Operate quitely"},
	{"debug",	0,	"",	"Debug mode"},
	{NULL}
};


OptionParser::Option_T	get_options[] = {
	{"ifile", Wasp::CvtToCPPStr, &opt.ifile, sizeof(opt.ifile)},
	{"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
	{"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
	{"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
	{NULL}
};

const char	*ProgName;

void make_params(DataMgrV3_0 *dataMgr, string parent_name, string outfile) {

	ofstream out(outfile);
	if (! out) {
		MyBase::SetErrMsg("Failed to open file %s : %M", outfile.c_str());
		exit (1);
	}

	ParamsBase::StateSave ssave;
	ParamsSeparator parent(&ssave, "TOP");

	TwoDDataParams dataParams(dataMgr, &ssave);
	dataParams.SetParent(&parent);

	vector <string> varnames = dataMgr->GetDataVarNames();
	for (int i=0; i<varnames.size(); i++) {
		dataParams.GetTransferFunc(varnames[i]);
	}
		


#ifdef	DEAD
	ParamsContainer tfs(&psm, "TransferFunctions");
	tfs.GetNode()->SetParent(&parent);

	TransferFunction tf1(&psm);
	TransferFunction tf2(&psm);
	TransferFunction tf3(&psm);

	// create a second opacity map
	//
	(void) tf3.createOpacityMap();

	tfs.Insert(&tf1, "X");
	tfs.Insert(&tf2, "Y");
	tfs.Insert(&tf3, "Z");

	

	ParamsContainer twoDparams(&psm, &parent, "ImageParams");

	TwoDImageParams imgparams1(&psm, NULL, 0);
	TwoDImageParams imgparams2(&psm, NULL, 0);

	TwoDImageParams imgparams3(imgparams2);
	imgparams3.SetHeightVariableName("hgt");

	twoDparams.Insert(&imgparams1, "img1");
	twoDparams.Insert(&imgparams2, "img2");
	twoDparams.Insert(&imgparams3, "img3");
#endif

	out << *(parent.GetNode());

	out.close();
}

void import_params(DataMgrV3_0 *dataMgr, string infile) {

	XmlNode parent;
	XmlParser parser;

	int rc = parser.LoadFromFile(&parent, infile);
	if (rc<0) exit(1);

	XmlNode *tfnode = parent.GetChild(0);
	if (! tfnode) return;

	ParamsBase::StateSave ssave;

	TwoDDataParams dataParams(dataMgr, &ssave, tfnode);

	cout << parent;
		
}

int main(int argc, char **argv) {

	OptionParser op;
	string	s;

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

	if (argc != 2) {
		cerr << "Usage: " << ProgName << " VDCMaster.nc [options] " << endl;
		op.PrintOptionHelp(stderr);
		exit(1);
	}

	string vdcfile = argv[1];
	vector <string> files(1, vdcfile);

	DataMgrV3_0 dataMgr("vdc", 1000);
	int rc = dataMgr.Initialize(files);
	if (rc<0) return(1);

	make_params(&dataMgr, "parent", "file.xml");

	import_params(&dataMgr, "file.xml");

	cout << "Allocated note count after delete : " <<
	XmlNode::GetAllocatedNodes().size() << endl;

}
