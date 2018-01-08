#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>

#include <vapor/OptionParser.h>
#include <vapor/CFuncs.h>
#include <vapor/VDCNetCDF.h>
#include <vapor/DCWRF.h>
#include <vapor/DCCF.h>
#include <vapor/DCMPAS.h>

using namespace Wasp;
using namespace VAPoR;


struct opt_t {
	int nthreads;
	int numts;
    std::vector <string> vars;
	OptionParser::Boolean_T	help;
} opt;

OptionParser::OptDescRec_T	set_opts[] = {
	{
		"nthreads",    1,  "0",    "Specify number of execution threads "
		"0 => use number of cores"
	},
	{
		"numts",    1,  "-1",
		"Number of timesteps to be included in the VDC. Default (-1) includes all timesteps."
	},
	{
		"vars",1, "",
		"Colon delimited list of 3D variable names (compressed) "
		"to be included in "
		"the VDC"
	},
	{"help",	0,	"",	"Print this message and exit"},
	{NULL}
};


OptionParser::Option_T	get_options[] = {
	{"nthreads",Wasp::CvtToInt,		&opt.nthreads,	sizeof(opt.nthreads)},
	{"numts",	Wasp::CvtToInt,		&opt.numts,		sizeof(opt.numts)},
	{"vars",	Wasp::CvtToStrVec,	&opt.vars,		sizeof(opt.vars)},
	{"help",	Wasp::CvtToBoolean,	&opt.help,		sizeof(opt.help)},
	{NULL}
};

string ProgName;

DC *DCCreate(string ftype) {
	if (ftype.compare("vdc") == 0) {
		return(new VDCNetCDF(opt.nthreads));
	} else if (ftype.compare("wrf") == 0) {
		return(new DCWRF());
	}
	else if (ftype.compare("cf") == 0) {
		return(new DCCF());
	}
	else if (ftype.compare("mpas") == 0) {
		return(new DCMPAS());
	}
	else {
		MyBase::SetErrMsg("Invalid data collection format : %s", ftype.c_str());
		return(NULL);
	}
}

float *Buffer1;
size_t BufSize1 = 0;
float *Buffer2;
size_t BufSize2 = 0;

void computeLMax(
	const float *buf1, const float *buf2, size_t nelements, 
	double &lmax, double &min, double &max
) {
	lmax = 0.0;
	min = 0.0;
	max = 0.0;
	if (nelements < 1) return;

	min = max = *buf1;
	
	for (size_t i=0; i<nelements; i++) {
		if (min > buf1[i]) {
			min = buf1[i];
		}
		if (max < buf1[i]) {
			max = buf1[i];
		}

		double diff = fabs(buf1[i] - buf2[i]);
		if (diff > lmax) {
			lmax = diff;
		}
	}
}

bool compare(DC *dc1, DC *dc2, size_t nts, string varname) {

	vector <size_t> dims, bs;
	int rc = dc1->GetDimLensAtLevel(varname, -1, dims, bs);
	if (rc<0) return(false);

	vector <size_t> dims2, bs2;
	rc = dc2->GetDimLensAtLevel(varname, -1, dims2, bs2);
	if (rc<0) return(false);

	assert(dims == dims2);

	size_t nelements = 1;
	for (int i=0; i<dims.size(); i++) {
		nelements *= dims[i];
	}

	if (BufSize1 < nelements) {
		if (Buffer1) delete [] Buffer1;
		Buffer1 = new float[nelements];
		BufSize1 = nelements;
		if (Buffer2) delete [] Buffer2;
		Buffer2 = new float[nelements];
		BufSize2 = nelements;
	}

	double nlmax_all = 0.0;
	for (int ts = 0; ts<nts; ts++) {
		rc = dc1->GetVar(ts, varname, -1, -1, Buffer1);
		if (rc<0) return(false);

		rc = dc2->GetVar(ts, varname, -1, -1, Buffer2);
		if (rc<0) return(false);

		double lmax, min, max;
		computeLMax(Buffer1, Buffer2, nelements, lmax, min, max);
		if ((max - min) != 0.0) {
			lmax /= (max-min);
		}
		if (lmax > nlmax_all) {
			nlmax_all = lmax;
		}
	}

	cout << "	NLmax = " << nlmax_all << endl;

	return(true);
}

int	main(int argc, char **argv) {

	OptionParser op;

	MyBase::SetErrMsgFilePtr(stderr);
	//
	// Parse command line arguments
	//
	ProgName = Basename(argv[0]);


	if (op.AppendOptions(set_opts) < 0) {
		exit(1);
	}

	if (op.ParseOptions(&argc, argv, get_options) < 0) {
		exit(1);
	}

	if (argc < 6) {
		cerr << "Usage: " << ProgName << "ftype1 ftype2 ftype1files... -- ftype2files " << endl;
		op.PrintOptionHelp(stderr, 80, false);
		exit(1);
	}
	

	if (opt.help) {
		cerr << "Usage: " << ProgName << " master.nc" << endl;
		op.PrintOptionHelp(stderr, 80, false);
		exit(0);
	}

	argc--;
	argv++;

	string ftype1 = argv[0];
	argc--;
	argv++;

	string ftype2 = argv[0];
	argc--;
	argv++;

	
	vector <string> files1;
	string sep("--");
	while (*argv && string(*argv) != sep) {
		files1.push_back(*argv);
		argv++;
	}
	if (*argv) argv++;

	vector <string> files2;
	while (*argv) {
		files2.push_back(*argv);
		argv++;
	}

	DC *dc1 = NULL;
	DC *dc2 = NULL;

	dc1 = DCCreate(ftype1);
	dc2 = DCCreate(ftype2);

	if (! dc1 || ! dc2) return(1);
	
	int rc = dc1->Initialize(files1, vector <string> ());
	if (rc<0) return(1);

	rc = dc2->Initialize(files2, vector <string> ());
	if (rc<0) return(1);

	vector <string> varnames;
	if (opt.vars.size()) { 
		varnames = opt.vars;
	}
	else {
		varnames = dc1->GetDataVarNames();
	}

	for (int i=0; i<varnames.size(); i++) {
		int nts = dc1->GetNumTimeSteps(varnames[i]);
		nts = opt.numts != -1 && nts > opt.numts ? opt.numts : nts;
		assert(nts >= 0);

		cout << "Testing variable " << varnames[i] << endl;

		bool ok = compare(dc1, dc2, nts, varnames[i]);
		if (! ok) {
			cout << "failed!" << endl;
		}
	}

	return(0);
}
