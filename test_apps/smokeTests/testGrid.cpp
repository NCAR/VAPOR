#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include "vapor/VAssert.h"

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
//#include <vapor/DataMgr.h>
#include <vapor/FileUtils.h>
//#include <vapor/utils.h>
#include "gridTools.h"

#include <vapor/ConstantGrid.h>

#include <float.h>
#include <cmath>

using namespace Wasp;
using namespace VAPoR;

namespace {
    size_t X = 0;
    size_t Y = 1;
    size_t Z = 2;

    std::vector< float* > Heap;
}

struct opt_t {
    std::vector <std::string> grids;
    std::vector <std::string> arrangements;
    std::vector <size_t> dims;
    std::vector <size_t> bs;
    std::vector <float> extents;
    double minValue;
    double maxValue;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T  set_opts[] = {
    {
        "grids", 1, "Regular:Stretched:Layered:Curvilinear",  
        "Colon delimited list of grids to test"
    },
    {
        "arrangements", 1, "Constant:Ramp:Triangle",  "Colon delimited list of "
        "data arrangements to test synthetic grids with"
    },
    {
        "dims", 1, "8:8:8",  "Data volume dimensions expressed in "
        "grid points (NX:NY:NZ)"
    },
    {
        "bs", 1, "64:64:64",
        "Internal storage blocking factor expressed in grid points (NX:NY:NZ)"
    },
    {
        "extents",  1,  "0:0:0:1:1:1",  "Colon delimited 6-element vector "
        "specifying domain extents of synthetic grids in user coordinates "
        "(X0:Y0:Z0:X1:Y1:Z1)"
    },
    {
        "minValue", 1,"0", 
        "The minimum data value to be assigned to cells in synthetic grids"
    },
    {
        "maxValue", 1,"1", 
        "The maximum data value to be assigned to cells in synthetic grids"
    },
    {"help",    0,  "", "Print this message and exit"},
    {nullptr}
};

OptionParser::Option_T  get_options[] = {
    {"grids", Wasp::CvtToStrVec, &opt.grids, sizeof(opt.grids)},
    {"arrangements", Wasp::CvtToStrVec, &opt.arrangements, sizeof(opt.arrangements)},
    {"dims", Wasp::CvtToSize_tVec, &opt.dims, sizeof(opt.dims)},
    {"bs", Wasp::CvtToSize_tVec, &opt.bs, sizeof(opt.bs)},
    {"extents", Wasp::CvtToFloatVec, &opt.extents, sizeof(opt.extents)},
    {"minValue", Wasp::CvtToDouble, &opt.minValue, sizeof(opt.minValue)},
    {"maxValue", Wasp::CvtToDouble, &opt.maxValue, sizeof(opt.maxValue)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {nullptr}
};
    
void InitializeOptions( int argc, char **argv, OptionParser &op ) {
    string  s;

    std::string ProgName = FileUtils::LegacyBasename(argv[0]);

    MyBase::SetErrMsgFilePtr(stderr);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }
   
    if (opt.extents.size() != 6 ) {
        cerr << "The -extents flag must contain 6 elements if used" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(1);
    }

    if (opt.dims.size() != 3) {
        cerr << "The -dims flag must contain 3 elements if used" << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    if (opt.help) {
        op.PrintOptionHelp(stderr);
        exit(0);
    }
}


int main( int argc, char** argv ) {
    double t0 = Wasp::GetTime();

    OptionParser op;
    InitializeOptions( argc, argv, op );

    std::cout << std::fixed << std::showpoint;
    std::cout << std::setprecision(4);

    std::vector< double > minu = { 
        opt.extents[X], 
        opt.extents[Y], 
        opt.extents[Z] 
    };
    std::vector< double > maxu = { 
        opt.extents[X+3], 
        opt.extents[Y+3], 
        opt.extents[Z+3] 
    };

    std::vector< size_t > bs2d = { opt.bs[X], opt.bs[Y] }; 
    std::vector< double > minu2d = { minu[X], minu[Y] };
    std::vector< double > maxu2d = { maxu[X], maxu[Y] };

    std::vector< size_t > dims2d = { opt.dims[X], opt.dims[Y] };

    // Test Regular Grid
    if ( std::find( opt.grids.begin(), opt.grids.end(), "Regular" ) != opt.grids.end() ) {
        std::vector< float* > rgBlks  = AllocateBlocks( opt.bs, opt.dims );
        RegularGrid* regularGrid = new RegularGrid( opt.dims, opt.bs, rgBlks, minu, maxu );
        TestGrid( regularGrid, opt.arrangements, opt.minValue, opt.maxValue );
        delete regularGrid;
    }

    // Test Stretched Grid
    if ( std::find( opt.grids.begin(), opt.grids.end(), "Stretched" ) != opt.grids.end() ) {
        StretchedGrid* stretchedGrid = MakeStretchedGrid( opt.dims, opt.bs, minu, maxu );
        TestGrid( stretchedGrid, opt.arrangements, opt.minValue, opt.maxValue );
        delete stretchedGrid;
    }

    // Test Layered Grid
    if ( std::find( opt.grids.begin(), opt.grids.end(), "Layered" ) != opt.grids.end() ) {
        LayeredGrid* layeredGrid = MakeLayeredGrid( opt.dims, opt.bs, minu, maxu );
        TestGrid( layeredGrid, opt.arrangements, opt.minValue, opt.maxValue );
        delete layeredGrid;
    }
   
    // Test Curvilinear Grid
    if ( std::find( opt.grids.begin(), opt.grids.end(), "Curvilinear" ) != opt.grids.end() ) {
        CurvilinearGrid* curvilinearGrid;
        curvilinearGrid = MakeCurvilinearTerrainGrid( opt.bs, minu, maxu, opt.dims);
        TestGrid( curvilinearGrid, opt.arrangements, opt.minValue, opt.maxValue );
        delete curvilinearGrid;
    }

    double t1 = Wasp::GetTime();
    cout << "Elapsed time: " << t1-t0 << endl;

    DeleteHeap();
}
