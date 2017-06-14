#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cerrno>
#include <cmath>
#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/WASP.h>

using namespace Wasp;
using namespace VAPoR;


//
//	Command line argument stuff
//
struct opt_t {
	string varname;
	string wname;
	int lod;
	int nthreads;
	std::vector <size_t> bs;
	std::vector <size_t> bs2d;
	std::vector <size_t> cratios;
	std::vector <size_t> cratios2d;
	std::vector <string> xvarnames;
	std::vector <string> xdimnames;
	OptionParser::Boolean_T	debug;
	OptionParser::Boolean_T	quiet;
	OptionParser::Boolean_T	help;
} opt;

OptionParser::OptDescRec_T	set_opts[] = {
	{"varname",	1, 	"var1",	"Name of variable"},
	{
		"wname", 1,"bior4.4", "Wavelet family used for compression "
		"Valid values are bior1.1, bior1.3, "
		"bior1.5, bior2.2, bior2.4 ,bior2.6, bior2.8, bior3.1, bior3.3, "
		"bior3.5, bior3.7, bior3.9, bior4.4"
	},
	{"lod",	1, 	"-1",	"Compression levels saved. 0 => coarsest, 1 => "
		"next refinement, etc. -1 => all levels defined by the netcdf file"},
	{"nthreads",	1, 	"0",	"Specify number of execution threads "
		"0 => use number of cores"},
	{
		"bs", 1, "64:64:64",
		"Internal storage blocking factor expressed in grid points (NZ:NY:NX) "
		"for 3D variables"
	},
	{
		"bs2d", 1, "",
		"Internal storage blocking factor expressed in grid points (NZ:NY:NX) "
		"for 2D variables. If empty the 2D blocking factor uses the fastest "
		"varying dimensions of the 3D blocking factor"
	},
	{
		"cratios",1, "500:100:10:1", "Colon delimited list of compression "
		"ratios for 3D variables. The default is 500:100:10:1. The maximum "
		"compression ratio is wavelet and block size dependent."
    },
	{
		"cratios2d",1, "", "Colon delimited list of compression "
		"ratios for 2D variables. If empty the 2D compression ratio vector "
		"is calculated from the 3D compression vector."
    },
	{
		"xvarnames",1, "", "Colon delimited list of variable names "
		"to exclude from compression."
    },
	{
		"xdimnames",1, "", "Colon delimited list of dimension names "
		"to exclude from compression."
    },
	{"debug",	0,	"",	"Enable diagnostic"},
	{"quiet",	0,	"",	"Operate quietly"},
	{"help",	0,	"",	"Print this message and exit"},
	{NULL}
};


OptionParser::Option_T	get_options[] = {
	{"varname", Wasp::CvtToCPPStr, &opt.varname, sizeof(opt.varname)},
	{"wname", Wasp::CvtToCPPStr, &opt.wname, sizeof(opt.wname)},
	{"lod", Wasp::CvtToInt, &opt.lod, sizeof(opt.lod)},
	{"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
	{"bs", Wasp::CvtToSize_tVec, &opt.bs, sizeof(opt.bs)},
	{"bs2d", Wasp::CvtToSize_tVec, &opt.bs2d, sizeof(opt.bs2d)},
	{"cratios", Wasp::CvtToSize_tVec, &opt.cratios, sizeof(opt.cratios)},
	{"cratios2d",Wasp::CvtToSize_tVec,&opt.cratios2d,sizeof(opt.cratios2d)},
    {"xvarnames", Wasp::CvtToStrVec, &opt.xvarnames, sizeof(opt.xvarnames)},
    {"xdimnames", Wasp::CvtToStrVec, &opt.xdimnames, sizeof(opt.xdimnames)},
	{"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
	{"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
	{"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
	{NULL}
};

const char	*ProgName;

// Return true if string 'name' contained in vector of strings, 'names'
//
bool name_in(string name, const vector <string> &names) {
	return(find(names.begin(), names.end(), name) != names.end());
}

// Given a list of dimemsion names and lengths (dimnames, dimlens) return
// an ordered subset of the inputs that contains only the dimension
// names and lenghts that will be compressed. 
//
void get_compressed_dims(
	const vector <string> &dimnames,
	const vector <size_t> &dimlens,
	vector <string> &cdimnames,
	vector <size_t> &cdimlens
) {
	assert(dimnames.size() == dimlens.size());
	cdimnames = dimnames;
	cdimlens = dimlens;

	// if any excluded dimension names match the **slowest**
	// varying variable dimension name remove the matched dimension
	// name.
	//
	vector <string>::iterator itr1 = cdimnames.begin();
	vector <size_t>::iterator itr2 = cdimlens.begin();
	while (itr1 != cdimnames.end()) {
		if (name_in(*itr1, opt.xdimnames)) {
			cdimnames.erase(itr1);
			itr1 = cdimnames.begin();

			cdimlens.erase(itr2);
			itr2 = cdimlens.begin();
		}
		else {
			break;
		}
	}
}

// Compute the block size from opt.bs and opt.bs2d based on the number 
// of dimensions
//
vector <size_t> get_bs(
	const vector <string> &dimnames, const vector <size_t> &dims
) {

	vector <string> cdimnames;
	vector <size_t> cdims;
	get_compressed_dims(dimnames, dims, cdimnames, cdims);

	assert(cdims.size() == 2 || cdims.size() == 3);

	if (cdims.size() == 3) return(opt.bs);

	if (cdims.size() == 2 && opt.bs2d.size()) return(opt.bs2d); 

	// Compute 2D block sizes from 3D block sizes
	//
	vector<size_t> bs = opt.bs;
	bs.erase(bs.begin());
	return(bs);
}

// Compute the compression ratios from opt.cratios and opt.cratios2d 
// based on the number of dimensions
//
vector <size_t> get_cratios(
	const vector <string> &dimnames, const vector <size_t> &dims
) {
	vector <string> cdimnames;
	vector <size_t> cdims;
	get_compressed_dims(dimnames, dims, cdimnames, cdims);

	assert(cdims.size() == 2 || cdims.size() == 3);

	if (cdims.size() == 3) return(opt.cratios);

	if (cdims.size() == 2 && opt.cratios2d.size()) return(opt.cratios2d); 

	// Compute 2D compression ratios from 3D compression ratios
	//
	vector<size_t> cratios = opt.cratios;
	for (int i=0; i<cratios.size(); i++) {
		double v = cratios[i];
		v = pow(v,1.0/3.0);
		cratios[i] = (size_t) (v * v);
	}
	return(cratios);
}
	
//
// Define the WASP output file using 'ncdf' as a template
//
int	DefFile(const NetCDFCpp &ncdf, WASP &wasp) {

	vector <string> dimnames;
	vector <size_t> dimlens;

	int rc = ncdf.InqDims(dimnames, dimlens);
	if (rc<0) return(-1);

	assert(dimnames.size() == dimlens.size());

	for (int i=0; i<dimnames.size(); i++) {
		rc = wasp.DefDim(dimnames[i], dimlens[i]);
		if (rc<0) return(-1);
	}

	vector <string> attnames;
	rc = ncdf.InqAttnames("", attnames);
	if (rc<0) return(-1);

	for (int i=0; i<attnames.size(); i++) {
		rc = ncdf.CopyAtt("", attnames[i], wasp, "");
		if (rc<0) return(-1);
	}

	return(0);
}


// Get all of the variable names, and based on command line arguments parse 
// the names into a vector of
// variable names that will be copied verbatim, and a vector of variable
// names that will be compressed.
//

int GetVarNames(
	const NetCDFCpp &ncdf,  vector <string> &vars,
	vector <string> &copy_vars, vector <string> &compress_vars
) {
	vars.clear();
	copy_vars.clear();
	compress_vars.clear();

	int rc = ncdf.InqVarnames(vars);
	if (rc<0) return(-1);

	for (int i=0; i<vars.size(); i++) {
		vector <string> dimnames;
		vector <size_t> dimlens;
		nc_type xtype;

		rc = ncdf.InqVarDims(vars[i], dimnames, dimlens);
		if (rc<0) return(-1);

		rc = ncdf.InqVartype(vars[i], xtype);
		if (rc<0) return(-1);

		// Can only compress 3 different data types
		//
		if (! ((xtype==NC_FLOAT) || (xtype==NC_DOUBLE) || (xtype==NC_INT))) {
			copy_vars.push_back(vars[i]);
			continue;
		}

		// Excluded variable names requested via command line
		//
		if (name_in(vars[i], opt.xvarnames)) {
			copy_vars.push_back(vars[i]);
			continue;
		}

		// if any excluded dimension names match the **slowest**
		// varying variable dimension name remove the matched dimension
		// name.
		//
		get_compressed_dims(dimnames, dimlens, dimnames, dimlens);

		if (! ((dimnames.size() == 2) || (dimnames.size() == 3))) {
			copy_vars.push_back(vars[i]);
			continue;
		}

		// If we get this far the variable can be compressed
		//
		compress_vars.push_back(vars[i]);
 
	}

	return(0);

}


// Copy variables verbatim from 'ncdf' to 'wasp'
//
int CopyVars(
	const NetCDFCpp &ncdf, const vector <string> &copy_vars, WASP &wasp
) {
	for (int i=0; i<copy_vars.size(); i++) {
		if (! opt.quiet) {
			cout << "Copying variable " << copy_vars[i] << endl;
		}
		int rc = ncdf.CopyVar(copy_vars[i], wasp);
		if (rc<0) return(-1);
	}
	return(0);
}

// Define a variable that will not be compressed (i.e copied verbatim)
//
int DefCopyVar(
	const NetCDFCpp &ncdf, string varname, NetCDFCpp &wasp
) {

	vector <string> dimnames;
	vector <size_t> dimlens;
	int rc = ncdf.InqVarDims(varname, dimnames, dimlens);
	if (rc<0) return(-1);

	nc_type xtype;
	rc = ncdf.InqVartype(varname, xtype);
	if (rc<0) return(-1);

	rc = wasp.DefVar(varname, xtype, dimnames);
	if (rc<0) return(-1);

	return(0);
}

// Define a variable that will be compressed 
//
int DefCompressVar(
	const NetCDFCpp &ncdf, string varname, WASP &wasp
) {

	vector <string> dimnames;
	vector <size_t> dimlens;
	int rc = ncdf.InqVarDims(varname, dimnames, dimlens);
	if (rc<0) return(-1);

	nc_type xtype;
	rc = ncdf.InqVartype(varname, xtype);
	if (rc<0) return(-1);

	vector <size_t> bs = get_bs(dimnames, dimlens);
	vector <size_t> cratios = get_cratios(dimnames, dimlens);
	rc = wasp.DefVar(varname, xtype, dimnames, opt.wname, bs, cratios);
	if (rc<0) return(-1);

	return(0);
}

// Define all variables in 'wasp', preserving the variable order 
// in 'ncdf'
//
int DefVars(
	const NetCDFCpp &ncdf, const vector <string> &vars, 
	const vector <string> &copy_vars, const vector <string> &comp_vars,
	WASP &wasp
) {
	int rc;
	for (int i=0; i<vars.size(); i++) {
		if (name_in(vars[i], copy_vars)) {
			rc = DefCopyVar(ncdf, vars[i], wasp);
			if (rc<0) return(-1);
		}
		else if (name_in(vars[i], comp_vars)) {
			rc = DefCompressVar(ncdf, vars[i], wasp);
			if (rc<0) return(-1);
		}
		else {
			continue;
		}
		// Now copy variable attributes
		//
		vector <string> attnames;
		rc = ncdf.InqAttnames(vars[i], attnames);
		if (rc<0) return(-1);

		for (int j=0; j<attnames.size(); j++) {
			rc = ncdf.CopyAtt(vars[i], attnames[j], wasp, vars[i]);
			if (rc<0) return(-1);
		}
	}
	return(0);
}


// Compress variables
//
int CompressVars(
	NetCDFCpp &ncdf, const vector <string> &copy_vars, WASP &wasp
) {
	for (int i=0; i<copy_vars.size(); i++) {
		if (! opt.quiet) {
			cout << "Compressing  variable " << copy_vars[i] << endl;
		}
		int rc = wasp.CopyVarFrom(copy_vars[i], ncdf);
		if (rc<0) return(-1);
	}
	return(0);
}

void Process(string ncdffile, string waspfile) {

	NetCDFCpp ncdf;
	WASP	wasp;
	size_t chunksize = 1024*1024*4;

	int rc = ncdf.Open(ncdffile, NC_NOWRITE);
	if (rc<0) {
		MyBase::SetErrMsg("Error opening %s for reading", ncdffile.c_str());
		exit(1);
	}

	rc = wasp.Create(
        waspfile, NC_64BIT_OFFSET, 0, chunksize, opt.cratios.size()
    );
	if (rc<0) {
		MyBase::SetErrMsg("Error opening %s for writing", waspfile.c_str());
		exit(1);
	}

	rc = DefFile(ncdf, wasp);
	if (rc < 0) exit(1);

	vector <string> vars, copy_vars, compress_vars;
	rc = GetVarNames(ncdf, vars, copy_vars, compress_vars);
	if (rc < 0) exit(1);

	rc = DefVars(ncdf, vars, copy_vars, compress_vars, wasp);
	if (rc < 0) exit(1);

	rc = wasp.EndDef();
	if (rc < 0) exit(1);

	rc = CopyVars(ncdf, copy_vars, wasp);
	if (rc < 0) exit(1);

	rc = CompressVars(ncdf, compress_vars, wasp);
	if (rc < 0) exit(1);

	(void) ncdf.Close();
	rc = wasp.Close();
	if (rc < 0) exit(1);

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

	if (opt.help) {
		cerr << "Usage: " << ProgName << " [options] netcdffile waspfile" << endl;
		op.PrintOptionHelp(stderr);
		exit(0);
	}

	if (argc != 3) {
		cerr << "Usage: " << ProgName << " [options] netcdffile waspfile" << endl;
		op.PrintOptionHelp(stderr);
		exit(1);
	}

	string ncdffile = argv[1];	// Path to a vdf file
	string waspfile = argv[2];	// Path to wasp file 

    if (opt.debug) MyBase::SetDiagMsgFilePtr(stderr);

	Process(ncdffile, waspfile);

	return(0);
}
