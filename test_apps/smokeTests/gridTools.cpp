#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/DataMgr.h>
#include <vapor/FileUtils.h>
#include <vapor/utils.h>

#include "vapor/VAssert.h"
#include "gridTools.h"

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

void DeleteHeap() {
    for (size_t i=0; i<Heap.size(); i++)
        delete [] Heap[i];
}

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

void MakeTriangle( Grid* grid, float minVal, float maxVal ) {
    std::vector< size_t > dims = grid->GetDimensions();
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
                grid->SetValueIJK( i, j, k, value );
            }
        }
    }
}

void MakeConstantField( Grid* grid, float value ) {
    std::vector< size_t > dims = grid->GetDimensions();
    size_t x = dims[X];
    size_t y = dims[Y];
    size_t z = 1;
    if ( dims.size() == 3 ) 
        z = dims[Z];

    for ( size_t k=0; k<z; k++ ) {
        for ( size_t j=0; j<y; j++ ) {
            for ( size_t i=0; i<x; i++ ) {
                grid->SetValueIJK( i, j, k, value );
            }
        }
    }
}

void MakeRamp( Grid* grid, float minVal, float maxVal ) {
    std::vector< size_t > dims = grid->GetDimensions();
    size_t x = dims[X];
    size_t y = dims[Y];
    size_t z = 1;
    if ( dims.size() == 3 ) 
        z = dims[Z];

    float increment = (maxVal - minVal) / ((x-1)*(y-1)*(z-1));

    float value = minVal;
    for ( size_t k=0; k<z; k++ ) {
        for ( size_t j=0; j<y; j++ ) {
            for ( size_t i=0; i<x; i++ ) {
                grid->SetValueIJK( i, j, k, value );
                value += increment;
            }
        }
    }
}

void MakeRampOnAxis( Grid* grid, float minVal, float maxVal, size_t axis=X ) {
    std::vector< size_t > dims = grid->GetDimensions();
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
                grid->SetValueIJK( i, j, k, value );
                value += xIncrement;
            }
            value += yIncrement;
        }
        value += zIncrement;
    }
}

// This function iterates across all nodes in a grid; making comparisons between the
// data values returned by the functions GetValueAtIndex() and GetValue().
int CompareIndexToCoords( 
    VAPoR::Grid* grid, 
    double &rms,                // Root Mean Square error
    size_t &numMissingValues,   // Counter for receiving MissingValue upon query
    size_t &disagreements      // Counter for when AccessIJK() and GetValue() disagree
) {
    rms   = 0.f;
    disagreements  = 0;
    numMissingValues = 0;

    std::vector< size_t > dims = grid->GetDimensions();
    size_t x = dims[X];
    size_t y = dims[Y];
    size_t z = 1;
    if ( dims.size() == 3 ) 
        z = dims[Z];
 
    double peak  = 0.f;
    double sum     = 0;
    for ( size_t k=0; k<z; k++ ) {
        for ( size_t j=0; j<y; j++ ) {
            for ( size_t i=0; i<x; i++ ) {
                size_t indices[3] = { i, j, k };
                //float trueValue = grid.AccessIJK( i, j, k );
                double trueValue = grid->GetValueAtIndex( {i,j,k} );
                
                double coords[3];
                grid->GetUserCoordinates( indices, coords );
                float sampleValue = grid->GetValue( coords );
                
                if (sampleValue == grid->GetMissingValue()) {
                    numMissingValues++;
                    continue;
                }

                double error = abs( sampleValue - trueValue );

                if (error != 0) {
                    disagreements++;
                }

                if ( error > peak ) 
                    peak = error;
                sum += error*error;

            }
        }
    }

    rms = sqrt( sum / (x*y*z) );

    if ( rms != 0 || disagreements > 0 )
        return 1;
    return 0;
}

size_t TestNodeIterator( const Grid *g, int& count, double& time ) {
    Grid::ConstNodeIterator itr;
    Grid::ConstNodeIterator enditr = g->ConstNodeEnd();

    double t0 = Wasp::GetTime();
    itr = g->ConstNodeBegin();

    for ( ; itr!=enditr; ++itr) {
        count++;
    }

    size_t expectedCount = 1;
    std::vector< size_t > dims = g->GetDimensions();
    for ( auto dim : dims )
        expectedCount *= dim;

    time = Wasp::GetTime() - t0;

    return expectedCount;
}

void PrintStats( double rms, size_t numMissingValues, size_t disagreements ) {
    cout << "    RMS error:                                     " << rms << endl;
    cout << "    Missing value count:                           " << numMissingValues << endl;
    cout << "    AccessIJK() vs GetValue() disagreement count:  " << disagreements << endl;
    cout << endl;
}

int RunTest(
    Grid* grid
) {
    int rc=0;
    double rms;
    size_t numMissingValues;
    size_t disagreements;

    if ( CompareIndexToCoords( grid, rms, numMissingValues, disagreements ) ) {
        cout << "       *** Error reported in " << grid->GetType() << " grid***" << endl;
        rc = 1;
    }

    if ( grid->GetInterpolationOrder() == 0 )
        cout << "  nearestNeighbor: " << endl;
    else
        cout << "  linear:          " << endl;
    PrintStats( rms, numMissingValues, disagreements );

    return rc;
}

int RunTests( 
    Grid* grid, 
    const std::vector<std::string> &tests, 
    float minVal, 
    float maxVal 
) {
//    double rms;
//    size_t numMissingValues;
//    size_t disagreements;

    std::vector< size_t > dims = grid->GetDimensions();
    size_t x = dims[X];
    size_t y = dims[Y];
    size_t z = 1;
    if ( dims.size() == 3 ) 
        z = dims[Z];

    int rc=0;
    std::string type = grid->GetType();
    
    cout << "=======================================================" << endl << endl;
    if ( std::find( tests.begin(), tests.end(), "Constant" ) != tests.end() ) {
        cout << type << " " << x << "x" << y << "x" << z << " Constant field:" << endl;
        MakeConstantField( grid, maxVal );
       
        grid->SetInterpolationOrder( linear );
        if ( RunTest( grid ) != 0 )
            rc = -1;
       
        grid->SetInterpolationOrder( nearestNeighbor );
        if ( RunTest( grid ) != 0 )
            rc = -1;
    }
  
    if ( std::find( tests.begin(), tests.end(), "Ramp" ) != tests.end() ) {
        cout << type << " " << x << "x" << y << "x" << z << " Ramp up through domain:" << endl;
        MakeRamp( grid, minVal, maxVal);
        
        grid->SetInterpolationOrder( linear );
        if ( RunTest( grid ) != 0 )
            rc = -1;
        
        grid->SetInterpolationOrder( nearestNeighbor );
        if ( RunTest( grid ) != 0 )
            rc = -1;
    }
    
    if ( std::find( tests.begin(), tests.end(), "RampOnAxis" ) != tests.end() ) {
        cout << type << " " << x << "x" << y << "x" << z << " Ramp up on Z axis:" << endl;
        MakeRampOnAxis( grid, minVal, maxVal, Z);
        grid->SetInterpolationOrder( linear );
        if ( RunTest( grid ) != 0 )
            rc = -1;

        grid->SetInterpolationOrder( nearestNeighbor );
        if ( RunTest( grid ) != 0 )
            rc = -1;
    }
    
    if ( std::find( tests.begin(), tests.end(), "Triangle" ) != tests.end() ) {
        cout << type << " " << x << "x" << y << "x" << z << " Triangle signal:" << endl;
        MakeTriangle( grid, minVal, maxVal );
       
        grid->SetInterpolationOrder( linear );
        if ( RunTest( grid ) != 0 )
            rc = -1;
        RunTest( grid );
        
        grid->SetInterpolationOrder( nearestNeighbor );
        if ( RunTest( grid ) != 0 )
            rc = -1;
    }

    int count = 0;
    double time;
    size_t expectedCount = TestNodeIterator( grid, count, time );
    if (expectedCount != count)
        rc = -1;

    cout << type << " Grid::ConstNodeIterator" << endl;
    cout << "  Count:          " << count << endl;;
    cout << "  Expected Count: " << expectedCount << endl;;
    cout << "  Time:  " << time << endl;
    cout << endl;

    return rc;
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
    MakeRampOnAxis(xrg, minu[X], maxu[X], X);

    std::vector <float *> yblks = AllocateBlocks(bs2d, dims2d);
    RegularGrid *yrg = new RegularGrid( dims2d, bs2d, yblks, minu2d, maxu2d );
    MakeRampOnAxis(yrg, minu[Y], maxu[Y], Y);

    std::vector <float *> zblks = AllocateBlocks(bs, dims);
    RegularGrid *zrg = new RegularGrid( dims, bs, zblks, minu, maxu );
    MakeRampOnAxis(zrg, minu[Z], maxu[Z], Z);

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
    MakeRampOnAxis( &rg, minu[Z], maxu[Z], Z );

    std::vector< float* > dataBlocks = AllocateBlocks( bs, dims );
    LayeredGrid *lg = new LayeredGrid(dims, bs, dataBlocks, hminu, hmaxu, rg);

    return(lg);
}

VAPoR::StretchedGrid* MakeStretchedGrid(
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
