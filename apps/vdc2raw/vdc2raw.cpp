#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cerrno>
#include <cassert>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/VDCNetCDF.h>

using namespace Wasp;
using namespace VAPoR;


struct opt_t {
	int	ts;
	string varname;
	string type;
	int	level;
	int	lod;
	int	nthreads;
	OptionParser::Boolean_T	help;
	OptionParser::Boolean_T	debug;
	OptionParser::Boolean_T	quiet;
	vector <int> xregion;
	vector <int> yregion;
	vector <int> zregion;
} opt;

OptionParser::OptDescRec_T	set_opts[] = {
	{"ts",		1, 	"0","Timestep of data file starting from 0"},
	{"varname",	1, 	"var1",	"Name of variable"},
	{"type",    1,  "float32",  "Primitive type of output data"},
	{"level",1, "-1","Multiresolution refinement level. Zero implies coarsest resolution"},
	{"lod",1, "-1","Compression level of detail. Zero implies coarsest approximation"},
	{"nthreads",1, "0","Number of execution threads (0=># processors)"},
	{"help",	0,	"",	"Print this message and exit"},
	{"debug",	0,	"",	"Enable debugging"},
	{"quiet",	0,	"",	"Operate quietly"},
	{"xregion", 1, "-1:-1", "X dimension subregion bounds (min:max)"},
	{"yregion", 1, "-1:-1", "Y dimension subregion bounds (min:max)"},
	{"zregion", 1, "-1:-1", "Z dimension subregion bounds (min:max)"},
	{NULL}
};


OptionParser::Option_T	get_options[] = {
	{"ts", Wasp::CvtToInt, &opt.ts, sizeof(opt.ts)},
	{"varname", Wasp::CvtToCPPStr, &opt.varname, sizeof(opt.varname)},
	{"type", Wasp::CvtToCPPStr, &opt.type, sizeof(opt.type)},
	{"level", Wasp::CvtToInt, &opt.level, sizeof(opt.level)},
	{"lod", Wasp::CvtToInt, &opt.lod, sizeof(opt.lod)},
	{"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
	{"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
	{"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
	{"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
	{"xregion", Wasp::CvtToIntVec, &opt.xregion, sizeof(opt.xregion)},
	{"yregion", Wasp::CvtToIntVec, &opt.yregion, sizeof(opt.yregion)},
	{"zregion", Wasp::CvtToIntVec, &opt.zregion, sizeof(opt.zregion)},
	{NULL}
};

size_t size_of_type(string type) {
	if (type.compare("float32") == 0) return(4);
	if (type.compare("float64") == 0) return(8);
	if (type.compare("int8") == 0) return(1);
	return(1);
}

int write_data(
	FILE	*fp, 
	string type,	// element type
	size_t n,
	const float *slice
) {

	static SmartBuf smart_buf;

	size_t element_sz = size_of_type(type);
	unsigned char *buffer = (unsigned char *) smart_buf.Alloc(n * element_sz);
	

	const float *sptr = slice;
	if (type.compare("float32")==0) {
		float *dptr = (float *) buffer;
		for (size_t i=0; i<n; i++) {
			*dptr++ = (float) *sptr++;
		}
	}
	else if (type.compare("float64")==0) {
		double *dptr = (double *) buffer;
		for (size_t i=0; i<n; i++) {
			*dptr++ = (double) *sptr++;
		}
	}
	else if (type.compare("int8")==0) {
		char *dptr = (char *) buffer;
		for (size_t i=0; i<n; i++) {
			*dptr++ = (char) *sptr++;
		}
	}

	int rc = fwrite(buffer, element_sz, n, fp);
	if (rc != n) {
		MyBase::SetErrMsg("Error writing output file : %M");
		return(-1);
	}

	return(0);
}


void process_volume(
	size_t ts,
	string varname,
	int level,
	int lod,
	VDCNetCDF &vdc,
	FILE *fp,
	vector <size_t> dims,
	string type
) {

	size_t nelements = 1;
	for (int i=0; i<dims.size() && i < 2; i++) {
		nelements *= dims[i];
	}
	float *buffer = new float[nelements];

	if (dims.size() < 2) {
		int rc = vdc.GetVar(ts, varname, level, lod, buffer);
		if (rc<0) exit(1);

		rc = write_data(fp, type, nelements, buffer);
		if (rc<0) exit(1);
	}
	else {
		int rc = vdc.OpenVariableRead(ts, varname, level, lod);
		if (rc<0) exit(1);

		size_t nz = dims.size() > 2 ? dims[2] : 1;

		for (size_t i=0; i<nz; i++) {
			rc = vdc.ReadSlice(buffer);
			if (rc<0) exit(1);

			rc = write_data(fp, type, nelements, buffer);
			if (rc<0) exit(1);
		}

		rc = vdc.CloseVariable();
		if (rc<0) exit(1);
	}

	delete [] buffer;

}

void process_region(
	size_t ts,
	string varname,
	int level,
	int lod,
	VDCNetCDF &vdc,
	FILE *fp,
	vector <size_t> dims,
	string type,
	const vector <int> &xregion,
	const vector <int> &yregion,
	const vector <int> &zregion
) {
	assert(dims.size() >= 1 && dims.size() <= 3); 
	assert(xregion.size() == 2);
	assert(yregion.size() == 2);
	assert(zregion.size() == 2);

	vector <size_t> min_bound;
	vector <size_t> max_bound;

	min_bound.push_back(xregion[0] < 0 ? 0 : xregion[0]);
	max_bound.push_back(xregion[1] < 0 ? dims[0]-1 : xregion[1]);

	int rc = vdc.OpenVariableRead(ts, varname, level, lod);
	if (rc<0) exit(1);

	if (dims.size() > 1) {
		min_bound.push_back(yregion[0] < 0 ? 0 : yregion[0]);
		max_bound.push_back(yregion[1] < 0 ? dims[1]-1 : yregion[1]);
	}
	if (dims.size() > 2) {
		min_bound.push_back(zregion[0] < 0 ? 0 : zregion[0]);
		max_bound.push_back(zregion[1] < 0 ? dims[2]-1 : zregion[1]);
	}

	size_t nelements = 1;
	for (int i=0; i<dims.size(); i++) {
		nelements *= max_bound[i] - min_bound[i] + 1;
	}

	float *region = new float[nelements];
	
	rc = vdc.ReadRegion(min_bound, max_bound, region);
	if (rc<0) exit(1);

	rc = write_data(fp, type, nelements, region);
	if (rc<0) exit(1);

	delete [] region;
	
	rc = vdc.CloseVariable();
	if (rc<0) exit(1);
}

const char	*ProgName;

int	main(int argc, char **argv) {

	OptionParser op;

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
		cerr << "Usage: " << ProgName << " [options] vdcmaster datafile" << endl;
		op.PrintOptionHelp(stderr);
		exit(0);
	}
	if (opt.xregion.size() != 2 || opt.yregion.size() != 2 || opt.zregion.size() != 2) {
		cerr << "Usage: " << ProgName << " [options] vdcmaster datafile" << endl;
		op.PrintOptionHelp(stderr);
		exit(1);
	}

	if (argc != 3) {
		cerr << "Usage: " << ProgName << " [options] vdcmaster datafile" << endl;
		op.PrintOptionHelp(stderr);
		exit(1);
	}

	string vdcmaster = argv[1];
	string datafile = argv[2];

	FILE *fp = fopen(datafile.c_str(), "w");
	if (! fp) {
		MyBase::SetErrMsg("Could not open file \"%s\" : %M", datafile.c_str());
		exit(1);
	}

	VDCNetCDF vdc(opt.nthreads);

	int rc = vdc.Initialize(vdcmaster, VDC::R, 4*1024*1024);
	if (rc<0) exit(1);

	vector <size_t> dims;
	vector <size_t> dummy;
	if (vdc.GetDimLensAtLevel(opt.varname, opt.level, dims, dummy) < 0) {
		exit(1);
	}


	if (dims.size() < 1) {
		MyBase::SetErrMsg("Variable must be 1D, 2D or 3D");
		exit(1);
	}


	//

	if (opt.xregion[0] == -1 && opt.xregion[1] == -1 && 
		opt.yregion[0] == -1 && opt.yregion[1] == -1 &&
		opt.zregion[0] == -1 && opt.zregion[1] == -1) { 

		process_volume(
			opt.ts, opt.varname, opt.level, opt.lod, vdc,
			fp, dims, opt.type
		);
	}
	else {
		process_region(
			opt.ts, opt.varname, opt.level, opt.lod, vdc, 
			fp, dims, opt.type, opt.xregion, opt.yregion, opt.zregion
		);
	}

	if (! opt.quiet) {
		cout << "Wrote ";
		for (int i=0;i<dims.size(); i++) {
			cout << dims[i];
			if (i!=dims.size()-1) cout << "x";
		}
		cout << endl;
	}


	fclose(fp);

	exit(0);
}
	



