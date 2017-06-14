//
//      $Id$
//
//***********************************************************************
//                                                                      *
//                      Copyright (C)  2006                             *
//          University Corporation for Atmospheric Research             *
//                      All Rights Reserved                             *
//                                                                      *
//***********************************************************************
//
//	File:		amr.cpp
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		Thu Apr  9 11:42:00 MDT 2009
//
//	Description:	This example demonstrates the construction of 
//	a VDC AMR data set. The example data are synthesized by sampling
//  a regular Cartesian grid. Prior to executing this code a suitable
//	.vdf file must be created using the command:
//
//		vdfcreate -gridtype block_amr -dimension NxNxN -bs 
//		CellDimxCellDimxCellDim -level NLevels 
//		-varnames ml:checker test.vdf
//
// where N, CellDim, and NLevels are the values defined below. For example:
//
//		vdfcreate -gridtype block_amr -dimension 256x256x256 -bs 
//		8x8x8 -level 2 	-varnames ml:checker test.vdf
//
// Once a .vdf file has been created, the example can be invoked with
// the command:
//
//		$VAPOR_HOME/bin/amr_ex test.vdf
//
//
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <cstdio>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/AMRTree.h>
#include <vapor/AMRData.h>
#include <vapor/AMRIO.h>
#include <vapor/Metadata.h>

using namespace Wasp;
using namespace VAPoR;

//
// Dimension of AMR grid if all cells are fully refined, creating
// a rectilinear Cartesian grid.
//
const int N = 256;		

//
// Dimension of AMR block. Must be at least 2. This restriction could
// be relaxed if AMRData::ReGrid were modified.
//
const int CellDim = 8;

//
// Maximum number of refinement levels. Must be hardcoded to 2 in this example
//
const int NLevels = 2;

//
//	Command line argument stuff
//
struct opt_t {
	OptionParser::Boolean_T	help;
	OptionParser::Boolean_T	debug;
} opt;

OptionParser::OptDescRec_T	set_opts[] = {
	{"help",	0,	"",	"Print this message and exit"},
	{"debug",	0,	"",	"Enable verbose debugging output"},
	{NULL}
};


OptionParser::Option_T	get_options[] = {
	{"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
	{"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
	{NULL}
};

//
// The marschner lobb function for synthesizing data
//
double  marschner_lobb(
    double  x,
    double  y,
    double  z,
    double  alpha,
    double  fm
) {
    double  pr;
    double  r;
    double  v;

    r = sqrt (x*x + y*y);
    pr = cos(2*M_PI*fm*cos(M_PI*r/2.0));

    v = (1.0 - sin(M_PI*z/2.0) + (alpha * (1.0 + pr))) / (2*(1+alpha));
    return(v);
}

#define ALPHA       (double) 0.25
#define FM      (double) 6.0
float *make_marschner_lobb(int nx, int ny, int nz) {

	float *grid = new float [nx*ny*nz];

    for(int z=0; z<nz; z++) {
    for(int y=0; y<ny; y++) {
    for(int x=0; x<nx; x++) {
        double xf, yf, zf;

        xf = (x - nx/2) * 2.0 / (double) (nx-1.0);
        yf = (y - ny/2) * 2.0 / (double) (ny-1.0);
        zf = (z - nz/2) * 2.0 / (double) (nz-1.0);

        grid[x + (nx*y) + (nx*ny*z)] =
            (float) marschner_lobb(xf, yf, zf, ALPHA, FM);
    }
    }
    }

	return(grid);
}

//
// another function for synthesizing data
//
#define ISODD(X)    ((X) % 2)
float *make_checker(int nx, int ny, int nz, int chksz) {

	float *grid = new float[nx*ny*nz];

    for(int z=0; z<nz; z++) {
    for(int y=0; y<ny; y++) {
    for(int x=0; x<nx; x++) {

		if (ISODD((x/chksz) + (y/chksz) + (z/chksz))) {
			grid[x + (nx*y) + (nx*ny*z)] = 1.0;
		}
		else {
			grid[x + (nx*y) + (nx*ny*z)] = 0.0;
		}
    }
    }
    }

	return(grid);
}

//
//	This function interpolates data on a 3D Cartesian grid using Nearest
//	Neighbor 
// 
float resample_grid_point(
	float *grid,
	double x, double y, double z
) {

	int xi, yi, zi;

	double delta = 1.0 / (double) N;
	double start = delta / 2.0;

	xi =  (int) rint((x-start) * (double) N);
	yi =  (int) rint((y-start) * (double) N);
	zi =  (int) rint((z-start) * (double) N);

	assert(xi >= 0 && xi < N);
	assert(yi >= 0 && yi < N);
	assert(zi >= 0 && zi < N);

	return(grid[zi*N*N + yi*N + xi]);
}



//
//	This function will write a single AMR variable to a Vapor Data 
//	Collection (VDC)
//
const char	*ProgName;
void	process_variable(
	AMRIO *amrio,
	AMRTree *tree,
	AMRData *amrdata,
	float *grid,
	const char *varname
) {

	AMRTree::cid_t cellid;
	size_t cell_dim[] = {CellDim, CellDim, CellDim};

	// 
	// Treverse the AMR tree hierarchy. For each cell in the tree, whether
	// it is a leaf or internal node, we provide data by sampling 
	// a function defined on a Cartesian grid.
	//
	bool first = true;
	while ((cellid = tree->GetNextCell(first)) >= 0) {
		first = false;

		double minu[3], maxu[3];

		// Get the bounds of the cell (min and max extent) defined in 
		// in user coordinates. In this example user coordinates run
		// from 0.0 to 1.0.
		//
		int rc = tree->GetCellBounds(cellid, minu, maxu);
		if (rc < 0) {
			cerr << ProgName << " : " << tree->GetErrMsg() << endl; 
			exit(1);
		}

		// Get a pointer to a block of data associated with this cell.
		//
		float *block = amrdata->GetBlock(cellid);

		// For each grid point in the cell block assign a value 
		//
		for (int k=0; k<cell_dim[2]; k++) {
		for (int j=0; j<cell_dim[1]; j++) {
		for (int i=0; i<cell_dim[0]; i++) {
			double deltax = (maxu[0]-minu[0]) / (double) cell_dim[0];
			double deltay = (maxu[1]-minu[1]) / (double) cell_dim[1];
			double deltaz = (maxu[2]-minu[2]) / (double) cell_dim[2];

			double startx = minu[0] + (deltax / 2.0);
			double starty = minu[1] + (deltay / 2.0);
			double startz = minu[2] + (deltaz / 2.0);

			block[k*cell_dim[0]*cell_dim[1] + j*cell_dim[0] + i] =  
			resample_grid_point(
				grid, startx + i*deltax, starty + j*deltay, startz + k*deltaz
			);
			
		}
		}
		}
	}

	//
	// Write the data to the VDC giving it the name, 'varname'
	//
	if (amrio->OpenVariableWrite(0, varname, -1) < 0) {
		cerr << ProgName << " : " << amrio->GetErrMsg() << endl;
		exit(1);
	}

	if (amrio->VariableWrite(amrdata) < 0) {
		cerr << ProgName << " : " << amrio->GetErrMsg() << endl;
		exit(1);
	}

	if (amrio->CloseVariable() < 0) {
		cerr << ProgName << " : " << amrio->GetErrMsg() << endl;
		exit(1);
	}

}
int main(int argc, char **argv) {

	OptionParser op;
    const char  *metafile;


	ProgName = Basename(argv[0]);

	if (op.AppendOptions(set_opts) < 0) {
		cerr << ProgName << " : " << op.GetErrMsg();
		exit(1);
	}

	if (op.ParseOptions(&argc, argv, get_options) < 0) {
		cerr << ProgName << " : " << op.GetErrMsg();
		exit(1);
	}

	if (opt.help) {
		cerr << "Usage : " << ProgName << " [options] metafile " << endl; 
		op.PrintOptionHelp(stderr);
		exit(0);
	}

	if (argc != 2) {
		cerr << "Usage : " << ProgName << " [options] metafile " << endl; 
		op.PrintOptionHelp(stderr);
		exit(1);
	}

	metafile = argv[1]; // Path to a suitably defined vdf file

	MyBase::SetErrMsgFilePtr(stderr);
	if (opt.debug) MyBase::SetDiagMsgFilePtr(stderr);

	// Create an AMRIO object to write the AMR grid to the VDC
	//
	AMRIO *amrio = new AMRIO(metafile);
	if (amrio->GetErrCode() != 0) {
		cerr << ProgName << " : " << amrio->GetErrMsg() << endl;
		exit(1);
	}

	//
	// Verify .vdf metafile was properly setup with vdfcreate command
	//
	const size_t *bs = amrio->GetBlockSize();
	for (int i=0; i<3; i++) {
		if (bs[i] != CellDim) {
			cerr << ProgName << " : Invalid block size" << endl;
			exit (1);
		}
	}
	const size_t *dim = amrio->GetDimension();
	for (int i=0; i<3; i++) {
		if (dim[i] != N) {
			cerr << ProgName << " : Invalid dimension : " << dim[i] << endl;
			exit (1);
		}
	}
	int nlevels = amrio->GetNumTransforms();
	if (nlevels != NLevels) {
		cerr << ProgName << " : Invalid number of refinment levels" << endl;
		exit (1);
	}

	// The dimensions of the base level AMR mesh specified in blocks (cells)
	size_t basedim = (N / CellDim) >> NLevels;
	assert (basedim >=5);	// we' going to refine the boundaries twice
	size_t basedimv[3] = {basedim,basedim,basedim};

	// User coordinate system extents. Can be set to anything
	//
	double min[3] = {0.0,0.0,0.0};
	double max[3] = {1.0,1.0,1.0};

	//
	// Create an AMRTree object to express the AMR refinement hierarchy
	//
	// The tree starts out as a rectilinear grid of cells with dimension
	// given by basedim
	//
	cout << ProgName << " : Tree create start" << endl;
	AMRTree *tree = new AMRTree(basedimv, min, max);
	if (AMRTree::GetErrCode() != 0) {
		fprintf(stderr, "AMRTree() : %s\n", AMRTree::GetErrMsg());
		exit(1);
	}

	// Refine the tree: The boundary cells are left unrefined. The cells
	// boardering the boundary cells are refined once. Everything else is 
	// refined twice.
	//
	for (AMRTree::cid_t z = 1; z<basedim-1; z++) {
	for (AMRTree::cid_t y = 1; y<basedim-1; y++) {
	for (AMRTree::cid_t x = 1; x<basedim-1; x++) {
		size_t xyz[] = {x,y,z};
			
		AMRTree::cid_t cellid = tree->GetCellID(xyz, 0);

		tree->RefineCell(cellid);
	}
	}
	}

	for (AMRTree::cid_t z = 2; z<basedim-2; z++) {
	for (AMRTree::cid_t y = 2; y<basedim-2; y++) {
	for (AMRTree::cid_t x = 2; x<basedim-2; x++) {
		size_t xyz[] = {x,y,z};

		AMRTree::cid_t cellid = tree->GetCellID(xyz, 0);
		AMRTree::cid_t child = tree->GetCellChildren(cellid);
			
		for (AMRTree::cid_t i = 0; i<8; i++) {
			tree->RefineCell(child+i);
		}
	}
	}
	}

	//
	// Calling EndRefinement() is optional, but it can signficantly 
	// improve performance,  namely when calling AMRData::GetBlock
	//
	tree->EndRefinement();

	cout << ProgName << " : Tree create finished" << endl;

	// Print out info about the tree hierarchy
	//
	if (opt.debug) {
		int	level;
		bool first = true;
		AMRTree::cid_t cellid;
		while ((cellid = tree->GetNextCell(first)) >= 0) {
			size_t xyz[3], xyz_b[3];
			AMRTree::cid_t baseblockidx;
			AMRTree::cid_t nodeidx;
			first = false;

			tree->GetCellLocation(cellid, xyz, &level);
			tree->DecodeCellID(cellid, &baseblockidx, &nodeidx);
			xyz_b[0] = xyz[0] >> level;
			xyz_b[1] = xyz[1] >> level;
			xyz_b[2] = xyz[2] >> level;
			cerr << "level, id, location base_location " 
				<< level << " (" << baseblockidx << " " << nodeidx << ") (" 
				<< xyz[0] << " " << xyz[1] << " " << xyz[2] << ") (" 
				<< xyz_b[0] << " " << xyz_b[1] << " " << xyz_b[2] << ")" 
				<< endl;
		}
	}

    //
    // Open a tree for writing at the indicated time step. Only one
	// tree is needed per time step (all variables at a given 
	// time step must have the same hierarchy).
    //
    if (amrio->OpenTreeWrite(0) < 0) {
        cerr << ProgName << " : " << amrio->GetErrMsg() << endl;
        exit(1);
    }

    if (amrio->TreeWrite(tree) < 0) {
        cerr << ProgName << " : " << amrio->GetErrMsg() << endl;
        exit(1);
    }

    if (amrio->CloseTree() < 0) {
        cerr << ProgName << " : " << amrio->GetErrMsg() << endl;
        exit(1);
    }

	// Create an AMRData object to store the sampled solution data. Each
	// cell in the AMR tree is of dimension CellDim by CellDim by CellDim
	//
	size_t cell_dim[] = {CellDim, CellDim, CellDim};
	AMRData *amrdata = new AMRData(tree, cell_dim);

	// Create a synthetic data set and place it in the tree
	//
	float *grid = make_marschner_lobb(N,N,N);
	cout << ProgName << " : variable 1 create start" << endl;
	process_variable(amrio, tree, amrdata, grid, "ml");
	cout << ProgName << " : variable 1 create finished" << endl;
	delete [] grid;

	grid = make_checker(N,N,N, 8);
	cout << ProgName << " : variable 2 create start" << endl;
	process_variable(amrio, tree, amrdata, grid, "checker");
	cout << ProgName << " : variable 2 create finished" << endl;
	delete [] grid;

	exit(0);
			
}
