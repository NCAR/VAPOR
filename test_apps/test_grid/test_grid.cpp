#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include "vapor/VAssert.h"

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/DataMgr.h>
#include <vapor/FileUtils.h>
#include <vapor/utils.h>
#include <vapor/GridHelper.h>

#include <vapor/ConstantGrid.h>

#include <float.h>
#include <cmath>

using namespace Wasp;
using namespace VAPoR;

enum interpOrder {
    nearestNeighbor = 0,
    linear          = 1
};

namespace {
    size_t X = 0;
    size_t Y = 1;
    size_t Z = 2;

    std::vector< float* > Heap;
}

struct opt_t {
    std::vector <size_t> dims;
    std::vector <size_t> bs;
    std::vector <float> extents;
    double minValue;
    double maxValue;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T  set_opts[] = {
    {
        "dims", 1, "64x64x64",  "Data volume dimensions expressed in "
        "grid points (NXxNYxNZ)"
    },
    {
        "bs", 1, "64:64:64",
        "Internal storage blocking factor expressed in grid points (NX:NY:NZ)"
    },
    {
        "extents",  1,  "0:0:0:1:1:1",  "Colon delimited 6-element vector "
        "specifying domain extents in user coordinates (X0:Y0:Z0:X1:Y1:Z1)"
    },
    {
        "minValue", 1,"0", 
        "The minimum data value to be assigned to cells in grid instances."
    },
    {
        "maxValue", 1,"1", 
        "The maximum data value to be assigned to cells in grid instances."
    },
    {"help",    0,  "", "Print this message and exit"},
    {nullptr}
};

OptionParser::Option_T  get_options[] = {
    {"dims", Wasp::CvtToSize_tVec, &opt.dims, sizeof(opt.dims)},
    {"bs", Wasp::CvtToSize_tVec, &opt.bs, sizeof(opt.bs)},
    {"extents", Wasp::CvtToFloatVec, &opt.extents, sizeof(opt.extents)},
    {"minValue", Wasp::CvtToDouble, &opt.minValue, sizeof(opt.minValue)},
    {"maxValue", Wasp::CvtToDouble, &opt.maxValue, sizeof(opt.maxValue)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {nullptr}
};
    
vector <float *> AllocateBlocks(
    const vector <size_t> &bs,
    const vector <size_t> &dims
) {

    size_t block_size = 1;
    size_t nblocks = 1;

    for (size_t i=0; i<bs.size(); i++) {

        block_size *= bs[i];

        VAssert(dims[i] > 0);
        size_t nb = ((dims[i] - 1) / bs[i]) + 1;

        nblocks *= nb;
    }

    float *buf = new float[nblocks * block_size];

    Heap.push_back(buf);

    std::vector <float *> blks;
    for (size_t i=0; i<nblocks; i++) {
        blks.push_back(buf + i*block_size);
    }

    return(blks);
}

void MakeTriangle( Grid& grid, float minVal, float maxVal ) {
    std::vector< size_t > dims = grid.GetDimensions();
    size_t x = dims[X];
    size_t y = dims[Y];
    size_t z = 1;
    if ( dims.size() == 3 ) 
        z = dims[Z];
    
    float value = minVal;
    for ( size_t k=0; k<z; k++ ) {
        for ( size_t j=0; j<y; j++ ) {
            for ( size_t i=0; i<x; i++ ) {
                value = value==minVal ? maxVal : minVal;
                grid.SetValueIJK( i, j, k, value );
            }
        }
    }
}

void MakeConstantField( Grid& grid, float value ) {
    std::vector< size_t > dims = grid.GetDimensions();
    size_t x = dims[X];
    size_t y = dims[Y];
    size_t z = 1;
    if ( dims.size() == 3 ) 
        z = dims[Z];

    for ( size_t k=0; k<z; k++ ) {
        for ( size_t j=0; j<y; j++ ) {
            for ( size_t i=0; i<x; i++ ) {
                grid.SetValueIJK( i, j, k, value );
            }
        }
    }
}

void MakeRamp( Grid& grid, float minVal, float maxVal, size_t axis=X ) {
    std::vector< size_t > dims = grid.GetDimensions();
    size_t x = dims[X];
    size_t y = dims[Y];
    size_t z = 1;
    if ( dims.size() == 3 ) 
        z = dims[Z];

    float xIncrement = axis==X ? (maxVal - minVal) / (dims[X]-1) : 0;
    float yIncrement = axis==Y ? (maxVal - minVal) / (dims[Y]-1) : 0;
    float zIncrement = axis==Z ? (maxVal - minVal) / (dims[Z]-1) : 0;

    float value = minVal;
    for ( size_t k=0; k<z; k++ ) {
        value = axis==Y ? minVal : value;      // reset value if we're ramping on Y
        for ( size_t j=0; j<y; j++ ) {
            value = axis==X ? minVal : value;  // reset value if we're ramping on X
            for ( size_t i=0; i<x; i++ ) {
                grid.SetValueIJK( i, j, k, value );
                value += xIncrement;
            }
            value += yIncrement;
        }
        value += zIncrement;
    }
}

void CompareIndexToCoords( 
    Grid& grid, 
    double &rms,                // Root Mean Square error
    size_t &numMissingValues,   // Counter for receiving MissingValue upon query
    size_t &disagreements,      // Counter for when AccessIJK() and GetValue() disagree
    bool nudge=false 
) {
    rms   = 0.f;
    disagreements  = 0;
    numMissingValues = 0;

    std::vector< size_t > dims = grid.GetDimensions();
    size_t x = dims[X];
    size_t y = dims[Y];
    size_t z = 1;
    if ( dims.size() == 3 ) 
        z = dims[Z];
 
    // Layered and Curvilinear grids have a bug where querying indices 
    // for the precise location of a grid node will return the wrong indices,
    // so we will nudge our coordinates for now.
    double xNudge = 0;
    double yNudge = 0;
    double zNudge = 0;

    if ( nudge == true ) {
        std::vector< double > minu, maxu;
        grid.GetUserExtents( minu, maxu );
        xNudge = FLT_MIN;//( maxu[X] - minu[X] ) / ( x*100 );
        yNudge = FLT_MIN;//( maxu[Y] - minu[Y] ) / ( y*100 );
        zNudge = FLT_MIN;//( maxu[Z] - minu[Z] ) / ( z*100 );
        xNudge = ( maxu[X] - minu[X] ) / ( x*10 );
        yNudge = ( maxu[Y] - minu[Y] ) / ( y*10 );
        zNudge = ( maxu[Z] - minu[Z] ) / ( z*10 );
    }

    double peak  = 0.f;

    double sum     = 0;
    for ( size_t k=0; k<z; k++ ) {
        if (k == z-1) zNudge *= -1;
        else zNudge = abs(zNudge);
        for ( size_t j=0; j<y; j++ ) {
            if (j == y-1) yNudge *= -1;
            else yNudge = abs(yNudge);
            for ( size_t i=0; i<x; i++ ) {
                if (i == x-1) xNudge *= -1;
                else xNudge = abs(xNudge);

                size_t indices[3] = { i, j, k };
                //float trueValue = grid.AccessIJK( i, j, k );
                double trueValue = grid.GetValueAtIndex( {i,j,k} );
                
                double coords[3];
                grid.GetUserCoordinates( indices, coords );
                coords[X] += xNudge;
                coords[Y] += yNudge;
                coords[Z] += zNudge;
                float sampleValue = grid.GetValue( coords );
                
                double error = abs( sampleValue - trueValue );

                if (error != 0) {
                    //cout << "     !!  "; 
                    disagreements++;
                }
                /*else cout << "         ";
                cout << "     " << i << " " << j << " " << k << endl;//" " << xNudge << " " << yNudge << " " << zNudge << endl;*/

                if (sampleValue == grid.GetMissingValue()) {
                    numMissingValues++;
                    continue;
                }

                if ( error > peak ) 
                    peak = error;
                sum += error*error;

                /*(cout << i << ":" << j << ":" << k << endl;
                cout << coords[0] << " " << coords[1] << " " << coords[2] << endl;*/
                //cout << trueValue << " " << sampleValue << endl << endl;
            }
        }
    }

    rms = sqrt( sum / (x*y*z) );
}

void PrintStats( double rms, size_t numMissingValues, size_t disagreements ) {
    cout << "    RMS error:                                     " << rms << endl;
    cout << "    Missing value count:                           " << numMissingValues << endl;
    cout << "    AccessIJK() vs GetValue() disagreement count:  " << disagreements << endl;
    cout << endl;
}

void TestGrid( Grid &grid, float minVal, float maxVal ) {
    double rms;
    size_t numMissingValues;
    size_t disagreements;

    std::vector< size_t > dims = grid.GetDimensions();
    size_t x = dims[X];
    size_t y = dims[Y];
    size_t z = 1;
    if ( dims.size() == 3 ) 
        z = dims[Z];

    std::string type = grid.GetType();

    cout << "=======================================================" << endl << endl;
    cout << type << " " << x << "x" << y << "x" << z << " Constant field RMS error:" << endl;
    MakeConstantField( grid, maxVal );
    grid.SetInterpolationOrder( linear );
    CompareIndexToCoords( grid, rms, numMissingValues, disagreements );
    cout << "  linear:          " << endl;
    PrintStats( rms, numMissingValues, disagreements );
    grid.SetInterpolationOrder( nearestNeighbor );
    CompareIndexToCoords( grid, rms, numMissingValues, disagreements );
    cout << "  nearestNeighbor: " << endl;
    PrintStats( rms, numMissingValues, disagreements );
    
    cout << type << " " << x << "x" << y << "x" << z << " Ramp up on Z axis RMS error:" << endl;
    MakeRamp( grid, minVal, maxVal, Z);
    grid.SetInterpolationOrder( linear );
    CompareIndexToCoords( grid, rms, numMissingValues, disagreements );
    cout << "  linear:          " << endl;
    PrintStats( rms, numMissingValues, disagreements );
    grid.SetInterpolationOrder( nearestNeighbor );
    CompareIndexToCoords( grid, rms, numMissingValues, disagreements, true );
    cout << "  nearestNeighbor: " << endl;
    PrintStats( rms, numMissingValues, disagreements );
    
    cout << type << " " << x << "x" << y << "x" << z << " Triangle signal RMS error:" << endl;
    MakeTriangle( grid, minVal, maxVal );
    grid.SetInterpolationOrder( linear );
    CompareIndexToCoords( grid, rms, numMissingValues, disagreements);
    cout << "  linear:          " << endl;
    PrintStats( rms, numMissingValues, disagreements );
    grid.SetInterpolationOrder( nearestNeighbor );
    CompareIndexToCoords( grid, rms, numMissingValues, disagreements, true );
    cout << "  nearestNeighbor: " << rms << endl;
    PrintStats( rms, numMissingValues, disagreements );
    cout << endl;
}   

VAPoR::CurvilinearGrid* MakeCurvilinearTerrainGrid( 
    const std::vector<size_t> &bs,
    const std::vector<double> &minu,
    const std::vector<double> &maxu,
    const std::vector<size_t> &dims
) {
    std::vector <size_t> bs2d   = { bs[X],   bs[Y] };
    std::vector <size_t> dims2d = { dims[X], dims[Y] };
    std::vector <double> minu2d = { minu[X], minu[Y] };
    std::vector <double> maxu2d = { maxu[X], maxu[Y] };

    std::vector <float *> xblks = AllocateBlocks(bs2d, dims2d);
    RegularGrid *xrg = new RegularGrid( dims2d, bs2d, xblks, minu2d, maxu2d );
    MakeRamp(*xrg, minu[X], maxu[X], X);

    std::vector <float *> yblks = AllocateBlocks(bs2d, dims2d);
    RegularGrid *yrg = new RegularGrid( dims2d, bs2d, yblks, minu2d, maxu2d );
    MakeRamp(*yrg, minu[Y], maxu[Y], Y);

    std::vector <float *> zblks = AllocateBlocks(bs, dims);
    RegularGrid *zrg = new RegularGrid( dims, bs, zblks, minu, maxu );
    MakeRamp(*zrg, minu[Z], maxu[Z], Z);

    std::vector <float *> blks = AllocateBlocks(bs, dims);
    CurvilinearGrid *cg = new CurvilinearGrid( dims, bs, blks, *xrg, *yrg, *zrg, NULL );

    return(cg);
}

LayeredGrid* MakeLayeredGrid(
    const vector <size_t> &dims,
    const vector <size_t > &bs,
    const std::vector<double> &minu,
    const std::vector<double> &maxu
) {
    // Get horizontal dimensions
    //
    std::vector <double> hminu = { minu[X], minu[Y] };
    std::vector <double> hmaxu = { maxu[X], maxu[Y] };

    std::vector< float* > zCoordBlocks = AllocateBlocks( bs, dims );

    RegularGrid rg( dims, bs, zCoordBlocks, minu, maxu );
    MakeRamp( rg, minu[Z], maxu[Z], Z );

    std::vector< float* > dataBlocks = AllocateBlocks( bs, dims );
    LayeredGrid *lg = new LayeredGrid(dims, bs, dataBlocks, hminu, hmaxu, rg);

    return(lg);
}

StretchedGrid* MakeStretchedGrid(
    const vector <size_t> &dims,
    const vector <size_t > &bs,
    const std::vector<double> &minu,
    const std::vector<double> &maxu
) {
    std::vector< double > xCoords( dims[X], 0 );
    std::vector< double > yCoords( dims[Y], 0 );
    std::vector< double > zCoords( dims[Z], 0 );

    double xRange = maxu[X] - minu[X];
    double yRange = maxu[Y] - minu[Y];
    double zRange = maxu[Z] - minu[Z];

    // Parabolically increasing coordinates
    for (size_t i=0; i<dims[X]; i++) {
        double xIncrement = xRange * pow( float(i)/(dims[X]-1), 2);
        xCoords[i] = xIncrement + minu[X];
    }
    for (size_t i=0; i<dims[Y]; i++) {
        double yIncrement = yRange * pow( float(i)/(dims[Y]-1), 2);
        yCoords[i] = yIncrement + minu[Y];
    }
    for (size_t i=0; i<dims[Z]; i++) {
        double zIncrement = zRange * pow( float(i)/(dims[Z]-1), 2.0);
        zCoords[i] = zIncrement + minu[Z];
    }

    vector <float *> blocks = AllocateBlocks( bs, dims );
    StretchedGrid* sg = new StretchedGrid( dims, bs, blocks, xCoords, yCoords, zCoords );
    return sg;
}

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
    std::vector< float* > rgBlks  = AllocateBlocks( opt.bs, opt.dims );
    RegularGrid regularGrid = RegularGrid( opt.dims, opt.bs, rgBlks, minu, maxu );
    TestGrid( regularGrid, opt.minValue, opt.maxValue );

    // Test Stretched Grid
    StretchedGrid* stretchedGrid = MakeStretchedGrid( opt.dims, opt.bs, minu, maxu );
    TestGrid( *stretchedGrid, opt.minValue, opt.maxValue );
    delete stretchedGrid;

    // Test Layered Grid
    LayeredGrid* layeredGrid = MakeLayeredGrid( opt.dims, opt.bs, minu, maxu );
    TestGrid( *layeredGrid, opt.minValue, opt.maxValue );
    delete layeredGrid;
   
    // Test Curvilinear Grid
    CurvilinearGrid* curvilinearGrid;
    curvilinearGrid = MakeCurvilinearTerrainGrid( opt.bs, minu, maxu, opt.dims);
    TestGrid( *curvilinearGrid, opt.minValue, opt.maxValue );
    delete curvilinearGrid;

    double t1 = Wasp::GetTime();
    cout << "Elapsed time: " << t1-t0 << endl;

    for (size_t i=0; i<Heap.size(); i++)
        delete [] Heap[i];
}

/*int main( int argc, char** argv ) {
    double t0 = Wasp::GetTime();

    OptionParser op;
    InitializeOptions( argc, argv, op );
    for (int i=0; i<opt.extents.size(); i++)
        cout << opt.extents[i] << endl;

    std::cout << std::fixed << std::showpoint;
    std::cout << std::setprecision(2);

    // User coordinates
    double minExt = 0.0;//-FLT_MAX;
    double maxExt = 1.0;// FLT_MAX;

    //std::vector< size_t > bs   = { 4, 4, 4 };
    std::vector< size_t > bs   = { 64, 64, 64 };
    std::vector< double > minu( 3, minExt);
    std::vector< double > maxu( 3, maxExt);

    std::vector< size_t > bs2d = { bs[0], bs[1] }; 
    std::vector< double > minu2d = { minu[0], minu[1] };
    std::vector< double > maxu2d = { maxu[0], maxu[1] };

    // Data value range
    //float minVal = -FLT_MAX/2;
    //float maxVal =  FLT_MAX/2;
    float minVal = -1000;
    float maxVal =  1000;

    // Grid discretization
    size_t x = 64;
    size_t y = 64;
    size_t z = 64;
    std::vector< size_t > dims =   { x,  y,  z };
    std::vector< size_t > dims2d = { x, y };

    // Test Regular Grid
    std::vector< float* > rgBlks  = AllocateBlocks( bs, dims );
    RegularGrid regularGrid = RegularGrid( dims, bs, rgBlks, minu, maxu );
    TestGrid( regularGrid, minVal, maxVal );

    // Test Stretched Grid
    StretchedGrid* stretchedGrid = MakeStretchedGrid( dims, bs, minu, maxu );
    TestGrid( *stretchedGrid, minVal, maxVal );
    delete stretchedGrid;

    // Test Layered Grid
    LayeredGrid* layeredGrid = MakeLayeredGrid( dims, bs, minu, maxu );
    TestGrid( *layeredGrid, minVal, maxVal );
    delete layeredGrid;
   
    // Test Curvilinear Grid
    CurvilinearGrid* curvilinearGrid = MakeCurvilinearTerrainGrid( bs, minu, maxu, dims);
    TestGrid( *curvilinearGrid, minVal, maxVal );
    delete curvilinearGrid;

    double t1 = Wasp::GetTime();
    cout << "Elapsed time: " << t1-t0 << endl;

    for (size_t i=0; i<Heap.size(); i++)
        delete [] Heap[i];
}*/
