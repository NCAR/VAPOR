/*  Wrappers for FLASH HDF Files
 */
#ifndef __FLASH_HDF__
#define __FLASH_HDF__

//#include <iostream.h>	//	DEPRECATED

#include <cmath>
#include <cstdio>
#include <cstring>

// HDF Header Files
#include <hdf5.h>

void HDF_ERROR_CHECK(int errorflag, char *message);

typedef char FlashVariableNames[5];

class FlashHDFFile {
public:
    // Default Constructor
    FlashHDFFile();
    // Constructor that opens a given HDF file
    FlashHDFFile(char *filename);

    // Open given HDF File
    int Open(char *filename);
    // Close currently opened HDF File
    int Close();

    // Get the number of dimensions in the current dataset {i.e. 1, 2, 3}
    int GetNumberOfDimensions();
    // Get the number of blocks in the current dataset
    int GetNumberOfBlocks();
    // Get the number of leaf blocks in the current dataset
    int GetNumberOfLeafs();

    // Get the number of cells per block
    void GetCellDimensions(int dimensions[3]);
    int  GetNodeType(int index);

    int GetRefineLevels(int refine_levels[]);

    int Get3dMinimumBounds(int index, double mbbox[3]);
    int Get3dMaximumBounds(int index, double mbbox[3]);

    // Spatial extent of dataset (Boundaries)
    int GetCoordinateRangeEntireDataset(double ranges[6]);

    // Spatial location of a given block (index)
    int Get3dCoordinate(int index, double coords[3]);

    // Size of a given block (index)
    int Get3dBlockSize(int index, double coords[3]);

    // Spatial location of a given block (index)
    //    int Get2dCoordinate(int index, double coords[2]);

    // Size of a given block (index)
    //    int Get2dBlockSize(int index, double coords[2]);

    int GetNumberOfVariables();
    int GetVariableNames(FlashVariableNames **names);

    // variableIndex is the number (1-23) chooses variable, index is block number
    //    int GetScalarVariable(int variableIndex, int index, double *variable);

    // Vector data is returned in an interleaved format (x,y,z,x,y,z) and
    // the the dimensions are automatically generated based on the
    // dimensionallity of the dataset
    //    int GetVectorVariable(int variableIndex, int index, double *variable);

    // Get scalar data -> variable name, index to block, pointer variable, runlength is for grabbing multiple blocks, bounds is spatial
    int GetScalarVariable(char variableName[5], int index, double *variable);
    int GetScalarVariable(char variableName[5], int index, int runlength, float *variable);
    int GetScalarVariable(char variableName[5], int index, double bounds[6], double *variable);
    int GetScalarVariable(char variableName[5], int index, int runlength, double bounds[6], double *variable);

    int GetRefinementLevel(int index);

    int GetGlobalIds(int index, int globalIds[]);
    int GetGlobalIds(int globalIds[]);

    int GetBoundingBoxes(float boxes[]);

    int GetNumberOfGlobalIds();

private:
    void _SetCellDimensions();
    void _InitSettings();
    void _ResetSettings(int constructor);

    hid_t  datasetId;    // Pointer to opened HDF5 File
    herr_t status;       // Commonly used return value

    int numberOfDimensions;    // Number of dimensions in the dataset
    int numberOfBlocks;
    int numberOfLeafs;

    double *min, *max;    // Minimum and maximum directions

    int cellDimensions[3];
};

#endif
