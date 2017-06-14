#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cerrno>
#include <cassert>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/WaveCodecIO.h>

using namespace Wasp;
using namespace VAPoR;


struct opt_t {
	int	ts;
	char *varname;
	int	lod;
	int	nthreads;
	OptionParser::Boolean_T	quiet;
	OptionParser::Boolean_T	help;
} opt;

OptionParser::OptDescRec_T	set_opts[] = {
	{"ts",		1, 	"0","Timestep of data file starting from 0"},
	{"varname",	1, 	"var1",	"Name of variable"},
	{"lod",1, "-1","Compression level of detail. Zero implies coarsest approximation"},
	{"nthreads",1, "0","Number of execution threads (0=># processors)"},
	{"quiet",	0,	"",	"Be less verbose"},
	{"help",	0,	"",	"Print this message and exit"},
	{NULL}
};


OptionParser::Option_T	get_options[] = {
	{"ts", Wasp::CvtToInt, &opt.ts, sizeof(opt.ts)},
	{"varname", Wasp::CvtToString, &opt.varname, sizeof(opt.varname)},
	{"lod", Wasp::CvtToInt, &opt.lod, sizeof(opt.lod)},
	{"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
	{"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
	{"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
	{NULL}
};

const char	*ProgName;


float *read_vdc_volume(
	string metafile, size_t dims[3], bool &has_missing, float &mv
) {
	has_missing = false;
	mv = 0.0;
	string varname = opt.varname;

	WaveCodecIO *wcreader = new WaveCodecIO(metafile, opt.nthreads);
	if (wcreader->GetErrCode() != 0) {
		return(NULL);
    }

	if (wcreader->OpenVariableRead(opt.ts, varname.c_str(),-1, opt.lod) < 0) {
		return(NULL);
	} 

    if (wcreader->GetVMissingValue(opt.ts, varname).size() == 1) {
		mv = wcreader->GetVMissingValue(0,varname)[0];
		has_missing = true;
    } else if (wcreader->GetTSMissingValue(opt.ts).size() == 1) {
        mv = wcreader->GetTSMissingValue(opt.ts)[0];
		has_missing = true;
    } else if (wcreader->GetMissingValue().size() == 1) {
        mv = wcreader->GetMissingValue()[0];
		has_missing = true;
    }

	Metadata::VarType_T vtype = wcreader->GetVarType(varname);
	wcreader->GetDim(dims, -1);

	switch (vtype) {
	case Metadata::VAR2D_XY:
		dims[2] = 1;
	break;

	case Metadata::VAR2D_XZ:
		dims[1] = dims[2];
		dims[2] = 1;
	break;
	case Metadata::VAR2D_YZ:
		dims[0] = dims[1]; dims[1] = dims[2]; dims[2] = 1;
	break;
	case Metadata::VAR3D:
	break;
	default:
	break;

	}
	
	size_t slice_size = dims[0] * dims[1];
	float *buf = new float[slice_size * dims[2]];
	float *bufptr = buf;
	assert (buf != NULL);

	for(int z = 0; z<dims[2]; z++) {

		if (! opt.quiet && z%10 == 0) {
			cout << "Reading vdc slice # " << z << endl; 
		}

		if (wcreader->ReadSlice(bufptr) < 0) {
			return(NULL);
		} 
		bufptr += slice_size;

	}
	wcreader->CloseVariable();

	return(buf);
}

float *read_raw_volume(string file, const size_t dims[3]) {

	FILE *fp = FOPEN64(file.c_str(), "r");
	if (! fp) {
		MyBase::SetErrMsg("Could not open file \"%s\" : %M", file.c_str());
		return(NULL);
	}

	size_t slice_size = dims[0] * dims[1];
	float *buf = new float[slice_size * dims[2]];
	float *bufptr = buf;
	assert (buf != NULL);

	for(int z = 0; z<dims[2]; z++) {

		if (! opt.quiet && z%10 == 0) {
			cout << "Reading raw slice # " << z << endl; 
		}

		int rc = fread(bufptr, sizeof(bufptr[0]), slice_size, fp);
		if (rc != slice_size) {
			MyBase::SetErrMsg("Errore reading slice : %M"); 
			return(NULL);
		}
		bufptr += slice_size;
	}

	fclose(fp);
	return(buf);
}


void compute_error(
    const float *odata, const float *cdata, size_t nelements,
    bool has_missing, float mv,
    float &lmax, float &rms,
	float &min, float &max
) {

    lmax = 0.0;
	rms = 0.0;
	min = 0.0;
	max = 0.0;

    double delta = 0;
	double sqrsum = 0;
	size_t nvalid = 0;

	bool first = true;
    for (size_t idx=0; idx<nelements; idx++) {
        if (has_missing && cdata[idx] == mv) continue;

		nvalid++;

		if (first) {
			min = max = odata[idx];
			first = false;
		}
		if (min>odata[idx]) min = odata[idx];
		if (max<odata[idx]) max = odata[idx];

        delta = fabs(odata[idx] - cdata[idx]);
        if (delta > lmax) lmax = delta;

        sqrsum += (odata[idx] - cdata[idx]) * (odata[idx] - cdata[idx]);
    }

	rms = (float) sqrt(sqrsum / (double) nvalid);
}


int	main(int argc, char **argv) {

	OptionParser op;
	string metafile;
	string datafile;

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
		cerr << "Usage: " << ProgName << " [options] metafile datafile" << endl;
		op.PrintOptionHelp(stderr);
		exit(0);
	}

	if (argc != 3) {
		cerr << "Usage: " << ProgName << " [options] metafile datafile" << endl;
		op.PrintOptionHelp(stderr);
		exit(1);
	}

	metafile = argv[1];
	datafile = argv[2];


	size_t dims[3];
	bool has_missing;
	float mv;
	float *vdcdata = read_vdc_volume(metafile, dims, has_missing, mv);
	if (! vdcdata) exit(1);

	float *rawdata = read_raw_volume(datafile, dims);
	if (! rawdata) exit(1);

	float lmax, rms, min, max;
	compute_error(
		rawdata, vdcdata, dims[0]*dims[1]*dims[2], has_missing, mv, 
		lmax, rms, min, max
	);
	float lmaxrel = 0.0;
	if ((max-min) != 0.0) lmaxrel = lmax / (max-min);
	float nrms = 0.0;
	if ((max-min) != 0.0) nrms = rms / (max-min);

    cout << "Range : " << min << " .. " << max << endl;
    cout << "Lmax : " << lmax << endl;
    cout << "Lmax Relative: " << lmaxrel << endl;
    cout << "RMS : " << rms << endl;
    cout << "Normalized RMS : " << nrms << endl;

	exit(0);
}

