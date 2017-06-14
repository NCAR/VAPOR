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
	int lod;
	int nthreads;
	std::vector <string> xvarnames;
	OptionParser::Boolean_T	debug;
	OptionParser::Boolean_T	quiet;
	OptionParser::Boolean_T	help;
} opt;

OptionParser::OptDescRec_T	set_opts[] = {
	{"varname",	1, 	"var1",	"Name of variable"},
	{"lod",	1, 	"-1",	"Compression levels saved. 0 => coarsest, 1 => "
		"next refinement, etc. -1 => all levels defined by the netcdf file"},
	{"nthreads",	1, 	"0",	"Specify number of execution threads "
		"0 => use number of cores"
	},
	{
		"xvarnames",1, "", "Colon delimited list of variable names "
		"to exclude from compression."
    },
	{"debug",	0,	"",	"Enable diagnostic"},
	{"quiet",	0,	"",	"Operate quietly"},
	{"help",	0,	"",	"Print this message and exit"},
	{NULL}
};


OptionParser::Option_T	get_options[] = {
	{"varname", Wasp::CvtToCPPStr, &opt.varname, sizeof(opt.varname)},
	{"lod", Wasp::CvtToInt, &opt.lod, sizeof(opt.lod)},
	{"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
    {"xvarnames", Wasp::CvtToStrVec, &opt.xvarnames, sizeof(opt.xvarnames)},
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

int GetDims(
	const WASP &wasp, vector <string> &dimnames, vector <size_t> &dimlens
) {
	dimnames.clear();
	dimlens.clear();

	vector <string> vars;
	int rc = wasp.InqVarnames(vars);
	if (rc<0) return(-1);

	// Ugh. No easy way to find all of the implicitly defined WASP
	// dimensions in a file. So find all explicitly defined 
	// dimensions for each variable, and use that list to build
	// list of dimensions
	//
	vector <string> all_dimnames;
	for (int i=0; i<vars.size(); i++) {
		vector <string> var_dimnames;
		vector <size_t> dummy;

		int rc = wasp.InqVarDims(vars[i], var_dimnames, dummy);
		if (rc<0) return(-1);

		all_dimnames.insert(
			all_dimnames.begin(), var_dimnames.begin(), var_dimnames.end()
		);
	}

    //
    // sort and remove duplicates
    //
    sort(all_dimnames.begin(), all_dimnames.end());
    vector <string>::iterator lasts;
    lasts = unique(all_dimnames.begin(), all_dimnames.end());
    all_dimnames.erase(lasts, all_dimnames.end());

	vector <string> my_dimnames;
	vector <size_t> my_dimlens;

	rc = wasp.InqDims(my_dimnames, my_dimlens);
	if (rc<0) return(-1);

	assert(my_dimnames.size() == my_dimlens.size());

	for (int i=0; i<my_dimnames.size(); i++) {
		if (name_in(my_dimnames[i], all_dimnames)) {
			dimnames.push_back(my_dimnames[i]);
			dimlens.push_back(my_dimlens[i]);
		}
	}
	return(0);
}
	
//
// Define the WASP output file using 'ncdf' as a template
//
int	DefFile(const WASP &wasp, NetCDFCpp &ncdf) {

	vector <string> dimnames;
	vector <size_t> dimlens;

	int rc = GetDims(wasp, dimnames, dimlens);
	if (rc<0) return(-1);

	assert(dimnames.size() == dimlens.size());

	for (int i=0; i<dimnames.size(); i++) {
		rc = ncdf.DefDim(dimnames[i], dimlens[i]);
		if (rc<0) return(-1);
	}

	vector <string> attnames;
	rc = wasp.InqAttnames("", attnames);
	if (rc<0) return(-1);

	for (int i=0; i<attnames.size(); i++) {

		if (attnames[i].find("WASP") != std::string::npos) continue;

		rc = wasp.CopyAtt("", attnames[i], ncdf, "");
		if (rc<0) return(-1);
	}

	return(0);
}


// Get all of the variable names, and parse 
// the names into a vector of
// variable names that will be copied verbatim, and a vector of variable
// names that will be de-compressed.
//

int GetVarNames(
	const WASP &wasp,  vector <string> &vars,
	vector <string> &copy_vars, vector <string> &compress_vars
) {
	vars.clear();
	copy_vars.clear();
	compress_vars.clear();

	int rc = wasp.InqVarnames(vars);
	if (rc<0) return(-1);

	for (int i=0; i<vars.size(); i++) {

		bool wasp_var;
		rc = wasp.InqVarWASP(vars[i], wasp_var);
		if (rc<0) return(-1);

		if (wasp_var) {
			compress_vars.push_back(vars[i]);
		}
		else {
			copy_vars.push_back(vars[i]);
		}
	}

	return(0);
}


// Copy variables verbatim from 'wasp' to 'ncdf'
//
int CopyVars(
	NetCDFCpp &wasp, const vector <string> &copy_vars, NetCDFCpp &ncdf
) {
	for (int i=0; i<copy_vars.size(); i++) {
		if (! opt.quiet) {
			cout << "Copying variable " << copy_vars[i] << endl;
		}
		int rc = wasp.CopyVar(copy_vars[i], ncdf);
		if (rc<0) return(-1);
	}
	return(0);
}

// Define a variable that will not be compressed (i.e copied verbatim)
//
int DefCopyVar(
	const WASP &wasp, string varname, NetCDFCpp &ncdf
) {

	vector <string> dimnames;
	vector <size_t> dimlens;
	int rc = wasp.InqVarDims(varname, dimnames, dimlens);
	if (rc<0) return(-1);

	nc_type xtype;
	rc = wasp.InqVartype(varname, xtype);
	if (rc<0) return(-1);

	rc = ncdf.DefVar(varname, xtype, dimnames);
	if (rc<0) return(-1);

	return(0);
}


// Define all variables in 'wasp', preserving the variable order 
// in 'ncdf'
//
int DefVars(
	const WASP &wasp, const vector <string> &vars, 
	NetCDFCpp &ncdf
) {
	int rc;
	for (int i=0; i<vars.size(); i++) {
		rc = DefCopyVar(wasp, vars[i], ncdf);
		if (rc<0) return(-1);

		// Now copy variable attributes
		//
		vector <string> attnames;
		rc = wasp.InqAttnames(vars[i], attnames);
		if (rc<0) return(-1);

		for (int j=0; j<attnames.size(); j++) {

			if (attnames[j].find("WASP") != std::string::npos) continue;

			rc = wasp.CopyAtt(vars[i], attnames[j], ncdf, vars[i]);
			if (rc<0) return(-1);
		}
	}
	return(0);
}


// Compress variables
//
int DeCompressVars(
	WASP &wasp, const vector <string> &copy_vars, NetCDFCpp &ncdf
) {
	for (int i=0; i<copy_vars.size(); i++) {
		if (! opt.quiet) {
			cout << "Decompressing  variable " << copy_vars[i] << endl;
		}
		int rc = wasp.CopyVarTo(copy_vars[i], ncdf);
		if (rc<0) return(-1);
	}
	return(0);
}

void Process(string waspfile, string ncdffile) {

	NetCDFCpp ncdf;
	WASP	wasp;
	size_t chunksize = 1024*1024*4;

	int rc = wasp.Open(waspfile, NC_NOWRITE);
	if (rc<0) {
		MyBase::SetErrMsg("Error opening %s for reading", waspfile.c_str());
		exit(1);
	}

	rc = ncdf.Create(ncdffile, NC_64BIT_OFFSET, 0, chunksize);
	if (rc<0) {
		MyBase::SetErrMsg("Error opening %s for writing", ncdffile.c_str());
		exit(1);
	}

	rc = DefFile(wasp, ncdf);
	if (rc < 0) exit(1);

	vector <string> vars, copy_vars, compress_vars;
	rc = GetVarNames(wasp, vars, copy_vars, compress_vars);
	if (rc < 0) exit(1);

	rc = DefVars(wasp, vars, ncdf);
	if (rc < 0) exit(1);

	rc = ncdf.EndDef();
	if (rc < 0) exit(1);

	rc = CopyVars(wasp, copy_vars, ncdf);
	if (rc < 0) exit(1);

	rc = DeCompressVars(wasp, compress_vars, ncdf);
	if (rc < 0) exit(1);

	(void) wasp.Close();
	rc = ncdf.Close();
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
		cerr << "Usage: " << ProgName << " [options] waspfile ncdffile" << endl;
		op.PrintOptionHelp(stderr);
		exit(0);
	}

	if (argc != 3) {
		cerr << "Usage: " << ProgName << " [options] waspfile ncdffile" << endl;
		op.PrintOptionHelp(stderr);
		exit(1);
	}

	string waspfile = argv[1];	// Path to wasp file 
	string ncdffile = argv[2];	// Path to a vdf file

    if (opt.debug) MyBase::SetDiagMsgFilePtr(stderr);

	Process(waspfile, ncdffile);

	return(0);
}
