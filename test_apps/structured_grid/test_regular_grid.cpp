#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cassert>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/RegularGrid.h>

using namespace Wasp;
using namespace VAPoR;


struct {
	std::vector <size_t> bs;
	std::vector <size_t> min;
	std::vector <size_t> max;
	std::vector <float> extents;
	std::vector <size_t> periodic;
	OptionParser::Boolean_T debug;
	OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T	set_opts[] = {
	{
		"bs",  1,  "64:64:64",  "Colon delimited 3-element vector "
		"specifying block size"
	},
	{
		"min",  1,  "0:0:0",  "Colon delimited 3-element vector "
		"specifying minimum voxel coordinates"
	},
	{
		"max",  1,  "127:127:127",  "Colon delimited 3-element vector "
		"specifying maximum voxel coordinates"
	},
	{
		"extents",  1,  "0:0:0:1:1:1",  "Colon delimited 6-element vector "
		"specifying domain extents in user coordinates (X0:Y0:Z0:X1:Y1:Z1)"
	},
	{
		"periodic",  1,  "0:0:0",  "Colon delimited 3-element vector "
		"of booleans specifying boundary periodicity"
	},
    {"debug",    0,  "", "Print diagnostics"},
    {"help",    0,  "", "Print this message and exit"},
	{NULL}
};


OptionParser::Option_T	get_options[] = {
	{"bs", Wasp::CvtToSize_tVec, &opt.bs, sizeof(opt.bs)},
	{"min", Wasp::CvtToSize_tVec, &opt.min, sizeof(opt.min)},
	{"max", Wasp::CvtToSize_tVec, &opt.max, sizeof(opt.max)},
	{"extents", Wasp::CvtToFloatVec, &opt.extents, sizeof(opt.extents)},
	{"periodic", Wasp::CvtToSize_tVec, &opt.periodic, sizeof(opt.periodic)},
	{"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
	{"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
	{NULL}
};

const char	*ProgName;

float *BlkBuf = NULL;

VAPoR::RegularGrid *make_regular_grid() {
	assert(opt.bs.size() == opt.min.size());
	assert(opt.bs.size() == opt.max.size());
	assert(opt.bs.size() == opt.extents.size() >> 1);
	assert(opt.bs.size() == opt.periodic.size());

	size_t block_size = 1;
	size_t nblocks = 1;

	size_t a_bs[] = {1,1,1};
	size_t a_min[] = {0,0,0};
	size_t a_max[] = {0,0,0};
	double a_extents[] = {0,0,0,0.0,0.0, 0.0};
	bool a_periodic[] = {0,0,0};
	for (int i=0; i<opt.bs.size(); i++) {
		a_bs[i] = opt.bs[i];
		a_min[i] = opt.min[i];
		a_max[i] = opt.max[i];
		a_periodic[i] = opt.periodic[i];

		a_extents[i] = opt.extents[i];
		a_extents[i+3] = opt.extents[i+3];

		block_size *= opt.bs[i];

		size_t b0 = opt.min[i] / opt.bs[i];
		size_t b1 = opt.max[i] / opt.bs[i];

		nblocks *= b1-b0+1;
	}

	BlkBuf = new float[nblocks * block_size];

    float **blkptrs = new float*[nblocks];
    for (int i=0; i<nblocks; i++) {
        if (blkptrs) blkptrs[i] = BlkBuf + i*block_size;
    }

	RegularGrid *rg = new RegularGrid(
		a_bs, a_min, a_max, a_extents, a_periodic, blkptrs
	);


	// Initialize data to linear ramp
	//
	size_t kmax = opt.max.size() >= 3 ? opt.max[2] - opt.min[2] + 1 : 1;
	size_t jmax = opt.max.size() >= 2 ? opt.max[1] - opt.min[1] + 1 : 1;
	size_t imax = opt.max.size() >= 1 ? opt.max[0] - opt.min[0] + 1 : 1;
	size_t index = 0;

	for (size_t k=0; k<kmax; k++) {
	for (size_t j=0; j<jmax; j++) {
	for (size_t i=0; i<imax; i++) {

		rg->AccessIJK(i,j,k) = (float) index;
		index++;

	}
	}
	}

	delete [] blkptrs;

	return(rg);
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

	if (opt.extents.size() && opt.extents.size() != 6) {
		cerr << "Usage: " << ProgName << " master.nc" << endl;
		op.PrintOptionHelp(stderr, 80, false);
		exit(1);
	}

	if (opt.help) {
		cerr << "Usage: " << ProgName << " [options] metafiles " << endl;
		op.PrintOptionHelp(stderr);
		exit(0);
	}

	if (opt.debug) {
		MyBase::SetDiagMsgFilePtr(stderr);
	}

	const RegularGrid *rg = make_regular_grid();
	if (! rg) return(1);

	size_t index = 0;
    RegularGrid::ConstIterator itr;
    for (itr = rg->begin(); itr!=rg->end(); ++itr) {
		assert(*itr == (float) index);
		index++;
    }

	index = 100033;
	itr = rg->begin() + index;
    for (; itr!=rg->end(); ++itr) {
		assert(*itr == (float) index);
		index++;
    }

	delete rg;

	delete [] BlkBuf;

	return(0);
}

	
