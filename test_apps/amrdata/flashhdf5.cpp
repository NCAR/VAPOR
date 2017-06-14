// General Headers
#include <iostream>
#include <cstdlib>
#include <climits>

// Class Headers
#include <flashhdf5.h>

// Utility Functions
#define CONSTRUCTOR 1
#define HDF_RETURN_CHECK(i) {if(i == -1) return 0;else return 1;}

using namespace std;

void
HDF_ERROR_CHECK(int errorflag, char *message)
{
    // HDF5 Returns a negative number on failure
    if(errorflag < 0)
    {
	cout << "ERROR: " << message << endl;
	exit(0);
    }// End of IF
}// End of HDF_ERROR_CHECK()

// Private Functions

void
FlashHDFFile::_ResetSettings(int constructor = 0)
{
    int i;

    numberOfLeafs = 0;
    numberOfDimensions = 0; // Number of dimensions in the dataset
    numberOfBlocks = 0;
    if(!constructor)
    {
	if(min != NULL)
	    delete [] min;
	if(max != NULL)
	    delete [] max;
    }// End of IF
    min = max = NULL;
    for(i=0;i<3;i++)
	cellDimensions[i] = -1;
    
}// End of _ResetSettings()

void
FlashHDFFile::_InitSettings()
{
//    cout << "in init" << endl;
    
    GetNumberOfBlocks();
//    cout << "Blocks finished " << endl;
    
    GetNumberOfLeafs();
//   cout << "leaves finished" << endl;
    
    GetNumberOfDimensions();
//    cout << "dims finished " << endl;
    
    _SetCellDimensions();
//    cout << "leaving init" << endl;
    
}// End of _initSettings(

// FlashHDFFile Public Class Functions

FlashHDFFile::FlashHDFFile()
{
    _ResetSettings(CONSTRUCTOR);
}// End of FlashHDFFile()

FlashHDFFile::FlashHDFFile(char *filename)
{
    _ResetSettings(CONSTRUCTOR);
//    cout << "About to open " << endl;
    
    datasetId = H5Fopen(filename,H5F_ACC_RDONLY,H5P_DEFAULT);
    HDF_ERROR_CHECK(datasetId,"ERROR: Failed On File Open");
    _InitSettings();
}// End of FlashHDFFile(char *filename)

int
FlashHDFFile::Open(char *filename)
{
    _ResetSettings(CONSTRUCTOR);
    datasetId = H5Fopen(filename,H5F_ACC_RDONLY,H5P_DEFAULT);
    HDF_ERROR_CHECK(datasetId,"ERROR: Failed On File Open");
    _InitSettings();
    return 1;
    
}// End of Open(char *filename)

int
FlashHDFFile::Close()
{
    _ResetSettings();
    H5Fclose(datasetId);
    return 1;
}// End of Close()

////
int 
FlashHDFFile::GetNumberOfDimensions()
{
    if(numberOfDimensions <= 0)
    {
#ifdef OLD
	int dimsizes[3]; // I don't think this should be hardcoded
	hid_t dataset = H5Dopen(datasetId, "number of zones per block");
	status = H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, dimsizes);
	for(int i=0;i<3;i++)
	{
	    cellDimensions[i] = dimsizes[i];
	    numberOfDimensions = ((dimsizes[i] > 0) ? (numberOfDimensions + 1) : numberOfDimensions);
	}// End of FOR
#else

	hid_t sp_type;
	
	typedef struct sim_params_t {
	    int total_blocks;
	    int nsteps;
	    int nxb;
	    int nyb;
	    int nzb;
	    double time; 
	    double timestep;
	    
	} sim_params_t;

	sim_params_t sim_params;
	
	hid_t dataset = H5Dopen(datasetId, "simulation parameters");

	/* create the HDF 5 compound data type to describe the record */
	sp_type = H5Tcreate(H5T_COMPOUND, sizeof(sim_params_t));
	
	H5Tinsert(sp_type, 
		  "total blocks", 
		  offsetof(sim_params_t, total_blocks),
		  H5T_NATIVE_INT);
 
	H5Tinsert(sp_type,
		  "time",
		  offsetof(sim_params_t, time),
		  H5T_NATIVE_DOUBLE);
	
	H5Tinsert(sp_type,
		  "timestep",
		  offsetof(sim_params_t, timestep),
		  H5T_NATIVE_DOUBLE);
	
	H5Tinsert(sp_type,
		  "number of steps",
		  offsetof(sim_params_t, nsteps),
		  H5T_NATIVE_INT);
	
	H5Tinsert(sp_type,
		  "nxb",
		  offsetof(sim_params_t, nxb),
		  H5T_NATIVE_INT);
	
	H5Tinsert(sp_type,
		  "nyb",
		  offsetof(sim_params_t, nyb),
		  H5T_NATIVE_INT);
	
	H5Tinsert(sp_type,
		  "nzb",
		  offsetof(sim_params_t, nzb),
		  H5T_NATIVE_INT);

  
	status = H5Dread(dataset, sp_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
			 &sim_params);

	H5Tclose(sp_type);
	H5Dclose(dataset);

//	cout << "X " << sim_params.nxb << " Y " << sim_params.nyb << " Z " << sim_params.nzb << endl;
	
	numberOfBlocks = sim_params.total_blocks;
	cellDimensions[0] = sim_params.nxb;
	cellDimensions[1] = sim_params.nyb;
	cellDimensions[2] = sim_params.nzb;
	for(int i=0;i<3;i++)
	{
	    numberOfDimensions = ((cellDimensions[i] > 0) ? (numberOfDimensions + 1) : numberOfDimensions);
	}// End of FOR
//	cout << "cX " << cellDimensions[0] << " cY " << cellDimensions[1] << " cZ " << cellDimensions[2] << endl;
    
#endif
    }// End of IF

    return numberOfDimensions;

}// End of GetNumberOfDimensions()

int
FlashHDFFile::GetNumberOfBlocks()
{
    if(numberOfBlocks <= 0)
    {
#if OLD
	hid_t dataset = H5Dopen(datasetId, "total blocks");
	status = H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &numberOfBlocks);
#else

	hid_t sp_type;
	
	typedef struct sim_params_t {
	    int total_blocks;
	    int nsteps;
	    int nxb;
	    int nyb;
	    int nzb;
	    double time; 
	    double timestep;
	    
	} sim_params_t;

	sim_params_t sim_params;
	
	hid_t dataset = H5Dopen(datasetId, "simulation parameters");

	/* create the HDF 5 compound data type to describe the record */
	sp_type = H5Tcreate(H5T_COMPOUND, sizeof(sim_params_t));
	
	H5Tinsert(sp_type, 
		  "total blocks", 
		  offsetof(sim_params_t, total_blocks),
		  H5T_NATIVE_INT);
 
	H5Tinsert(sp_type,
		  "time",
		  offsetof(sim_params_t, time),
		  H5T_NATIVE_DOUBLE);
	
	H5Tinsert(sp_type,
		  "timestep",
		  offsetof(sim_params_t, timestep),
		  H5T_NATIVE_DOUBLE);
	
	H5Tinsert(sp_type,
		  "number of steps",
		  offsetof(sim_params_t, nsteps),
		  H5T_NATIVE_INT);
	
	H5Tinsert(sp_type,
		  "nxb",
		  offsetof(sim_params_t, nxb),
		  H5T_NATIVE_INT);
	
	H5Tinsert(sp_type,
		  "nyb",
		  offsetof(sim_params_t, nyb),
		  H5T_NATIVE_INT);
	
	H5Tinsert(sp_type,
		  "nzb",
		  offsetof(sim_params_t, nzb),
		  H5T_NATIVE_INT);

  
	status = H5Dread(dataset, sp_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
			 &sim_params);

	H5Tclose(sp_type);
	H5Dclose(dataset);

	//	cout << "X " << sim_params.nxb << " Y " << sim_params.nyb << " Z " << sim_params.nzb << endl;
	
	numberOfBlocks = sim_params.total_blocks;
	cellDimensions[0] = sim_params.nxb;
	cellDimensions[1] = sim_params.nyb;
	cellDimensions[2] = sim_params.nzb;
//	cout << "cX " << cellDimensions[0] << " cY " << cellDimensions[1] << " cZ " << cellDimensions[2] << endl;
    
#endif
    }// End of IF

    return numberOfBlocks;
}// End of GetNumberOfBlocks

#define LEAF_NODE 1

int
FlashHDFFile::GetNumberOfLeafs()
{
    if(numberOfLeafs <= 0)
    {
	numberOfLeafs = 0;
	if(numberOfBlocks <= 0)
	    GetNumberOfBlocks();
	int i;
	for(i=0;i<numberOfBlocks;i++)
	    if(GetNodeType(i) == LEAF_NODE)
		numberOfLeafs++;
    }// End of IF
    
    return numberOfLeafs;
}// End of GetNumberOfLeafs()

int
FlashHDFFile::GetNumberOfGlobalIds()
{
    if(numberOfDimensions <= 0)
    {
	this->GetNumberOfDimensions();
	return ((2*numberOfDimensions) + (int)powf(2.0, (double)numberOfDimensions) + 1);
    }// End of IF
    else
	return ((2*numberOfDimensions) + (int)powf(2.0, (double)numberOfDimensions) + 1);
    
}// End of GetNumberOfGlobalIds()

int
FlashHDFFile::GetGlobalIds(int idx,int gids[])
{
    hid_t dataspace, dataset, memspace;
    int rank;
    hsize_t dimens_1d, dimens_2d[2];
	herr_t ierr;
    
    hsize_t start_2d[2];
    hsize_t stride_2d[2], count_2d[2];
    
    
    int i;
    for(i=0;i<15;i++)
	gids[i] = -2;

    rank = 2;
    dimens_2d[0] = numberOfBlocks;
    dimens_2d[1] = 15; // WRONG Hardcoded value;
    
    /* define the dataspace -- as described above */
    start_2d[0]  = (hssize_t)idx;
    start_2d[1]  = 0;
    
    stride_2d[0] = 1;
    stride_2d[1] = 1;
    
    count_2d[0]  = 1;
    count_2d[1]  = 15; // WRONG Hardcoded value;
    
    
    dataspace = H5Screate_simple(rank, dimens_2d, NULL);

    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
			       start_2d, stride_2d, count_2d, NULL);

    /* define the memory space */
    rank = 1;
    dimens_1d = 15; // WRONG Hardcoded value;
    memspace = H5Screate_simple(rank, &dimens_1d, NULL);
    
    dataset = H5Dopen(datasetId, "gid");
    status  = H5Dread(dataset, H5T_NATIVE_INT, memspace, dataspace, 
		      H5P_DEFAULT, gids);
    

#ifdef	DEAD
    for(i=0;i<count_2d[1];i++)
    {
	if(gids[i] >= 1)
	    gids[i] = gids[i]-1;
    }// End of FOR
#endif

    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);

    return 1;
}// End of GetGlobalIds(int idx,int gids[])

int
FlashHDFFile::GetGlobalIds(int gids[])
{
    hid_t dataspace, dataset, memspace;
    int rank;
    hsize_t dimens_1d, dimens_2d[2];
	herr_t ierr;
    
    hsize_t start_2d[2];
    hsize_t stride_2d[2], count_2d[2];
    
    
    rank = 2;
    dimens_2d[0] = numberOfBlocks;
    dimens_2d[1] = 15; // WRONG Hardcoded value;
    
    /* define the dataspace -- as described above */
    start_2d[0]  = 0;
    start_2d[1]  = 0;
    
    stride_2d[0] = 1;
    stride_2d[1] = 1;
    
    count_2d[0]  = numberOfBlocks;
    count_2d[1]  = 15; // WRONG Hardcoded value;
    
    
    dataspace = H5Screate_simple(rank, dimens_2d, NULL);

    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
			       start_2d, stride_2d, count_2d, NULL);

    /* define the memory space */
    rank = 1;
    dimens_1d = 15 * numberOfBlocks; // WRONG Hardcoded value;
    memspace = H5Screate_simple(rank, &dimens_1d, NULL);
    
    dataset = H5Dopen(datasetId, "gid");
    status  = H5Dread(dataset, H5T_NATIVE_INT, memspace, dataspace, 
		      H5P_DEFAULT, gids);
    


    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);

    return 1;
}// End of GetGlobalIds(int gids[])


int
FlashHDFFile::GetBoundingBoxes(float bboxes[])
{
    hid_t dataspace, dataset, memspace;
    int rank;
    hsize_t dimens_1d, dimens_3d[3];
	herr_t ierr;
    
    hsize_t start_3d[3];
    hsize_t stride_3d[3], count_3d[3];
    
    rank = 3;
    dimens_3d[0] = numberOfBlocks;
    dimens_3d[1] = 3; // WRONG Hardcoded value;
    dimens_3d[2] = 2; // WRONG Hardcoded value;
    
    /* define the dataspace -- as described above */
    start_3d[0]  = 0;
    start_3d[1]  = 0;
    start_3d[2]  = 0;
    
    stride_3d[0] = 1;
    stride_3d[1] = 1;
    stride_3d[2] = 1;
    
    count_3d[0]  = numberOfBlocks;
    count_3d[1]  = 3; // WRONG Hardcoded value;
    count_3d[2]  = 2; // WRONG Hardcoded value;
    
    dataspace = H5Screate_simple(rank, dimens_3d, NULL);

    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
			       start_3d, stride_3d, count_3d, NULL);

    /* define the memory space */
    rank = 1;
    dimens_1d = 6 * numberOfBlocks; // WRONG Hardcoded value;
    memspace = H5Screate_simple(rank, &dimens_1d, NULL);
    
    dataset = H5Dopen(datasetId, "bounding box");
    status  = H5Dread(dataset, H5T_NATIVE_FLOAT, memspace, dataspace, 
		      H5P_DEFAULT, bboxes);
    


    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);

    return 1;
}




// Gets the dimensions of an individual block, for viz its usually 9x9x9
//
void
FlashHDFFile::GetCellDimensions(int dimensions[3])
{

//    if(cellDimensions[0] == -1 || cellDimensions[1] == -1 || cellDimensions[2] == -1)
//    {
#ifdef OLD
	int dimsizes[3]; // I don't think this should be hardcoded
	hid_t dataset = H5Dopen(datasetId, "number of zones per block");
	status = H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, dimsizes);
	for(int i=0;i<3;i++)
	{
	    cellDimensions[i] = dimsizes[i];
	}// End of FOR
#else

	hid_t sp_type;
	
	typedef struct sim_params_t {
	    int total_blocks;
	    int nsteps;
	    int nxb;
	    int nyb;
	    int nzb;
	    double time; 
	    double timestep;
	    
	} sim_params_t;

	sim_params_t sim_params;
	
	hid_t dataset = H5Dopen(datasetId, "simulation parameters");

	/* create the HDF 5 compound data type to describe the record */
	sp_type = H5Tcreate(H5T_COMPOUND, sizeof(sim_params_t));
	
	H5Tinsert(sp_type, 
		  "total blocks", 
		  offsetof(sim_params_t, total_blocks),
		  H5T_NATIVE_INT);
 
	H5Tinsert(sp_type,
		  "time",
		  offsetof(sim_params_t, time),
		  H5T_NATIVE_DOUBLE);
	
	H5Tinsert(sp_type,
		  "timestep",
		  offsetof(sim_params_t, timestep),
		  H5T_NATIVE_DOUBLE);
	
	H5Tinsert(sp_type,
		  "number of steps",
		  offsetof(sim_params_t, nsteps),
		  H5T_NATIVE_INT);
	
	H5Tinsert(sp_type,
		  "nxb",
		  offsetof(sim_params_t, nxb),
		  H5T_NATIVE_INT);
	
	H5Tinsert(sp_type,
		  "nyb",
		  offsetof(sim_params_t, nyb),
		  H5T_NATIVE_INT);
	
	H5Tinsert(sp_type,
		  "nzb",
		  offsetof(sim_params_t, nzb),
		  H5T_NATIVE_INT);

  
	status = H5Dread(dataset, sp_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
			 &sim_params);

	H5Tclose(sp_type);
	H5Dclose(dataset);

//	cout << "NX " << sim_params.nxb << " NY " << sim_params.nyb << " NZ " << sim_params.nzb << endl;
	
	numberOfBlocks = sim_params.total_blocks;
	cellDimensions[0] = sim_params.nxb;
	cellDimensions[1] = sim_params.nyb;
	cellDimensions[2] = sim_params.nzb;	
#endif
//    }// End of IF
    
    dimensions[0] = cellDimensions[0];
    dimensions[1] = cellDimensions[1];
    dimensions[2] = cellDimensions[2];
    
//    cout << "cX " << cellDimensions[0] << " cY " << cellDimensions[1] << " cZ " << cellDimensions[2] << endl;
//    cout << "DX " << dimensions[0] << " DY " << dimensions[1] << " DZ " << dimensions[2] << endl;
    
    return;
}// End of GetNumberOfCellDimensions()

void
FlashHDFFile::_SetCellDimensions()
{
    if((cellDimensions[0] == -1) || (cellDimensions[1] == -1)
       || (cellDimensions[2] == -1))
    {
	int dimsizes[3];
	// I don't think this should be hardcoded
	hid_t dataset = H5Dopen(datasetId, "number of zones per block");
	
	status = H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, 
			 H5P_DEFAULT, dimsizes);
	
	for(int i=0;i<3;i++)
	{
	    cellDimensions[i] = dimsizes[i];
	}// End of FOR
    }// End of IF

    return;
}


int
FlashHDFFile::GetNodeType(int idx)
{
    int ntype;
    
    int rank;
    hsize_t dimens_1d;
    hid_t ierr;
    
    hid_t dataspace, memspace, dataset;
    
    hsize_t start_1d;
    
    hsize_t stride_1d, count_1d;
    
    herr_t status;

    rank = 1;
    dimens_1d = numberOfBlocks;

    /* define the dataspace -- same as above */
    start_1d  = (hssize_t)idx;
    stride_1d = 1;
    count_1d  = 1;
    
    dataspace = H5Screate_simple(rank, &dimens_1d, NULL);

    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
			     &start_1d, &stride_1d, &count_1d, NULL);
    
    /* define the memory space */
    dimens_1d = 1;
    memspace = H5Screate_simple(rank, &dimens_1d, NULL);
    
    /* create the dataset from scratch only if this is our first time */
    dataset = H5Dopen(datasetId, "node type");
    status  = H5Dread(dataset, H5T_NATIVE_INT, memspace, dataspace, 
		      H5P_DEFAULT, (void *)&ntype);
  
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);
    
    return ntype;
    
}// End of GetNodeType(int idx)

int
FlashHDFFile::GetRefineLevels(int refine_levels[])
{
    
    int rank;
    hsize_t dimens_1d;
    hid_t ierr;
    
    hid_t dataspace, memspace, dataset;
    
    hsize_t start_1d;
    
    hsize_t stride_1d, count_1d;
    
    herr_t status;

    rank = 1;
    dimens_1d = numberOfBlocks;

    /* define the dataspace -- same as above */
    start_1d  = 0;
    stride_1d = 1;
    count_1d  = numberOfBlocks;
    
    dataspace = H5Screate_simple(rank, &dimens_1d, NULL);

    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
			     &start_1d, &stride_1d, &count_1d, NULL);
    
    /* define the memory space */
    dimens_1d = numberOfBlocks;
    memspace = H5Screate_simple(rank, &dimens_1d, NULL);
    
    /* create the dataset from scratch only if this is our first time */
    dataset = H5Dopen(datasetId, "refine level");
    status  = H5Dread(dataset, H5T_NATIVE_INT, memspace, dataspace, 
		      H5P_DEFAULT, (void *)refine_levels);
  
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);
    
    return 1;
    
}// End of GetNodeType(int idx)

int
FlashHDFFile::GetRefinementLevel(int idx)
{
    int refinelevel;
    
    int rank;
    hsize_t dimens_1d;
    hid_t ierr;
    
    hid_t dataspace, memspace, dataset;
    
    hsize_t start_1d;
    hsize_t stride_1d, count_1d;
    
    herr_t status;

    rank = 1;
    dimens_1d = numberOfBlocks;

    /* define the dataspace -- same as above */
    start_1d  = (hssize_t)idx;
    stride_1d = 1;
    count_1d  = 1;
    
    dataspace = H5Screate_simple(rank, &dimens_1d, NULL);

    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
			     &start_1d, &stride_1d, &count_1d, NULL);
    
    /* define the memory space */
    dimens_1d = 1;
    memspace = H5Screate_simple(rank, &dimens_1d, NULL);
    
    /* create the dataset from scratch only if this is our first time */
    dataset = H5Dopen(datasetId, "refine level");
    status  = H5Dread(dataset, H5T_NATIVE_INT, memspace, dataspace, 
		      H5P_DEFAULT, (void *)&refinelevel);
  
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);
    
    return refinelevel;
}

#define nguard (0)// 4

int
FlashHDFFile::GetScalarVariable(char variableName[5], int dataPointIndex, double *variable)
{
    int rank;
    hsize_t dimens_4d[4];
    hid_t ierr;
    
    hid_t dataspace, memspace, dataset;
    
    hsize_t start_4d[5];
    hsize_t stride_4d[5], count_4d[5];
    
    herr_t status;
    
    
    /* ------------------------== unknowns ==--------------------------- */
    if((cellDimensions[0] <= -1) || (cellDimensions[1] <= -1) || 
       (cellDimensions[2] <= -1))
    {
	_SetCellDimensions(); // Cell dimensions have not been set.
    }// End of IF
    	
    rank = 4;
    dimens_4d[0] = numberOfBlocks;    
    dimens_4d[1] = cellDimensions[2];
    dimens_4d[2] = cellDimensions[1];
    dimens_4d[3] = cellDimensions[0];

  /* define the dataspace -- as described above */
    start_4d[0]  = (hssize_t)dataPointIndex;
    start_4d[1]  = 0;
    start_4d[2]  = 0;
    start_4d[3]  = 0;
    
    stride_4d[0] = 1;
    stride_4d[1] = 1;
    stride_4d[2] = 1;
    stride_4d[3] = 1;
    
    count_4d[0]  = 1;
    count_4d[1]  = cellDimensions[2];
    count_4d[2]  = cellDimensions[1];
    count_4d[3]  = cellDimensions[0];
    
    dataspace = H5Screate_simple(rank, dimens_4d, NULL);

    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
			       start_4d, stride_4d, count_4d, NULL);

    int k3d,k2d;
    k3d = k2d = 1;// Hard coded bad bad bad
    
    /* 
       define the memory space -- the unkt array that was passed includes all
       the variables and guardcells for a single block.  We want to cut out the
       portion that just has the interior cells and the variable specified by 
       index
    */
    rank = 4;
    dimens_4d[0] = cellDimensions[2]+(nguard)*2*k3d;
    dimens_4d[1] = cellDimensions[1]+(nguard)*2*k2d;
    dimens_4d[2] = cellDimensions[0]+(nguard)*2;
    dimens_4d[3] = 1; // Number of Variables 1

    memspace = H5Screate_simple(rank, dimens_4d, NULL);

    /* exclude the guardcells and take only the desired variable */
    start_4d[0] = (nguard)*k3d;
    start_4d[1] = (nguard)*k2d;  
    start_4d[2] = (nguard);
    start_4d[3] = 0;//variableIndex;  /* should be 0 *//* remember: 0 based indexing */
    
    stride_4d[0] = 1;
    stride_4d[1] = 1;  
    stride_4d[2] = 1;
    stride_4d[3] = 1;
    
    count_4d[0] = cellDimensions[2];
    count_4d[1] = cellDimensions[1]; 
    count_4d[2] = cellDimensions[0]; 
    count_4d[3] = 1; 
    
    double *unknowns = new double[cellDimensions[2]*cellDimensions[1]*cellDimensions[0]]; // This is bad
    
    ierr = H5Sselect_hyperslab(memspace, H5S_SELECT_SET,
			       start_4d, stride_4d, count_4d, NULL);
    
    dataset = H5Dopen(datasetId,variableName);
    status  = H5Dread(dataset, H5T_NATIVE_DOUBLE, memspace, dataspace, 
		      H5P_DEFAULT, unknowns);
    

    int i;

    // This seems bad
    for(i=0;i<cellDimensions[0]*cellDimensions[1]*cellDimensions[2];i++)
	variable[i] = (double)unknowns[i];


    delete [] unknowns;
    
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);

    return 1;
    
}// End of

int
FlashHDFFile::GetScalarVariable(char variableName[5], int dataPointIndex, double bounds[6], double *variable)
{

    int rank;
    hsize_t dimens_4d[4];
    hid_t ierr;
    
    hid_t dataspace, memspace, dataset;
    
    hsize_t start_4d[5];
    hsize_t stride_4d[5], count_4d[5];
    
    herr_t status;
    
    
    /* ------------------------== unknowns ==--------------------------- */
    if((cellDimensions[0] <= -1) || (cellDimensions[1] <= -1) || 
       (cellDimensions[2] <= -1))
    {
	_SetCellDimensions(); // Cell dimensions have not been set.
    }// End of IF
    	
    rank = 4;
    dimens_4d[0] = numberOfBlocks;    
    dimens_4d[1] = cellDimensions[2];
    dimens_4d[2] = cellDimensions[1];
    dimens_4d[3] = cellDimensions[0];

  /* define the dataspace -- as described above */
    start_4d[0]  = (hssize_t)dataPointIndex;
    start_4d[1]  = 0;
    start_4d[2]  = 0;
    start_4d[3]  = 0;
    
    stride_4d[0] = 1;
    stride_4d[1] = 1;
    stride_4d[2] = 1;
    stride_4d[3] = 1;
    
    count_4d[0]  = 1;
    count_4d[1]  = cellDimensions[2];
    count_4d[2]  = cellDimensions[1];
    count_4d[3]  = cellDimensions[0];
    
    dataspace = H5Screate_simple(rank, dimens_4d, NULL);

    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
			       start_4d, stride_4d, count_4d, NULL);

    int k3d,k2d;
    k3d = k2d = 1;// Hard coded bad bad bad
    
    /* 
       define the memory space -- the unkt array that was passed includes all
       the variables and guardcells for a single block.  We want to cut out the
       portion that just has the interior cells and the variable specified by 
       index
    */
    rank = 4;
    dimens_4d[0] = cellDimensions[2]+(nguard)*2*k3d;
    dimens_4d[1] = cellDimensions[1]+(nguard)*2*k2d;
    dimens_4d[2] = cellDimensions[0]+(nguard)*2;
    dimens_4d[3] = 1; // Number of Variables 1

    memspace = H5Screate_simple(rank, dimens_4d, NULL);

    /* exclude the guardcells and take only the desired variable */
    start_4d[0] = (nguard)*k3d;
    start_4d[1] = (nguard)*k2d;  
    start_4d[2] = (nguard);
    start_4d[3] = 0;//variableIndex;  /* should be 0 *//* remember: 0 based indexing */
    
    stride_4d[0] = 1;
    stride_4d[1] = 1;  
    stride_4d[2] = 1;
    stride_4d[3] = 1;
    
    count_4d[0] = cellDimensions[2];
    count_4d[1] = cellDimensions[1]; 
    count_4d[2] = cellDimensions[0]; 
    count_4d[3] = 1; 
    
    double *unknowns = new double[cellDimensions[0]*cellDimensions[1]*cellDimensions[2]]; 
    
    ierr = H5Sselect_hyperslab(memspace, H5S_SELECT_SET,
			       start_4d, stride_4d, count_4d, NULL);
    
    dataset = H5Dopen(datasetId,variableName);
    status  = H5Dread(dataset, H5T_NATIVE_DOUBLE, memspace, dataspace, 
		      H5P_DEFAULT, unknowns);
    
    int i;
    
    for(i=0;i<cellDimensions[0]*cellDimensions[1]*cellDimensions[2];i++)
	variable[i] = (double)unknowns[i];


    double minbounds[3],maxbounds[3];
    Get3dMinimumBounds(dataPointIndex,(double *)&minbounds);
    Get3dMaximumBounds(dataPointIndex,(double *)&maxbounds);

    bounds[0] = (double)minbounds[0];
    bounds[1] = (double)maxbounds[0];
    bounds[2] = (double)minbounds[1];
    bounds[3] = (double)maxbounds[1];    
    bounds[4] = (double)minbounds[2];
    bounds[5] = (double)maxbounds[2];
    
    delete [] unknowns;
    
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);

    return 1;
    
}// End of

int
FlashHDFFile::GetScalarVariable(char variableName[5], int dataPointIndex, int sizeofrun, float *variable)
{
//    cout << "Loading Variable " << variableName << endl;
    int rank;
    hsize_t dimens_4d[4];
    hid_t ierr;
    
    hid_t dataspace, memspace, dataset;
    
    hsize_t start_4d[5];
    hsize_t stride_4d[5], count_4d[5];
    
    herr_t status;
    
    
    /* ------------------------== unknowns ==--------------------------- */
    if((cellDimensions[0] <= -1) || (cellDimensions[1] <= -1) || 
       (cellDimensions[2] <= -1))
    {
	_SetCellDimensions(); // Cell dimensions have not been set.
    }// End of IF
    	
    rank = 4;
    dimens_4d[0] = numberOfBlocks;    
    dimens_4d[1] = cellDimensions[2]; 
    dimens_4d[2] = cellDimensions[1];
    dimens_4d[3] = cellDimensions[0];

  /* define the dataspace -- as described above */
    start_4d[0]  = (hssize_t)dataPointIndex;
    start_4d[1]  = 0;
    start_4d[2]  = 0;
    start_4d[3]  = 0;
    
    stride_4d[0] = 1;
    stride_4d[1] = 1;
    stride_4d[2] = 1;
    stride_4d[3] = 1;
    
    count_4d[0]  = sizeofrun;// was 1, now the number of blocks grabbed
    count_4d[1]  = cellDimensions[2];
    count_4d[2]  = cellDimensions[1];
    count_4d[3]  = cellDimensions[0];
    
    dataspace = H5Screate_simple(rank, dimens_4d, NULL);

    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
			       start_4d, stride_4d, count_4d, NULL);

    int k3d,k2d;
    k3d = k2d = 1;// Hard coded bad bad bad
    
    /* 
       define the memory space -- the unkt array that was passed includes all
       the variables and guardcells for a single block.  We want to cut out the
       portion that just has the interior cells and the variable specified by 
       index
    */
    rank = 5;
    dimens_4d[0] = numberOfBlocks;
    dimens_4d[1] = cellDimensions[2]+(nguard)*2*k3d;
    dimens_4d[2] = cellDimensions[1]+(nguard)*2*k2d;
    dimens_4d[3] = cellDimensions[0]+(nguard)*2;
    dimens_4d[4] = 1; // Number of Variables 1

    memspace = H5Screate_simple(rank, dimens_4d, NULL);

    /* exclude the guardcells and take only the desired variable */
    start_4d[0] = 0;
    start_4d[1] = (nguard)*k3d;
    start_4d[2] = (nguard)*k2d;  
    start_4d[3] = (nguard);
    start_4d[4] = 0;//variableIndex;  /* should be 0 *//* remember: 0 based indexing */
    
    stride_4d[0] = 1;
    stride_4d[1] = 1;  
    stride_4d[2] = 1;
    stride_4d[3] = 1;
    stride_4d[4] = 1;
    
    count_4d[0] = sizeofrun;
    
    count_4d[1] = cellDimensions[2];
    count_4d[2] = cellDimensions[1]; 
    count_4d[3] = cellDimensions[0]; 
    count_4d[4] = 1; 
    
    // This is bad why hard coded ...
    float *unknowns = new float[sizeofrun*cellDimensions[2]*cellDimensions[1]*cellDimensions[0]];
    
    ierr = H5Sselect_hyperslab(memspace, H5S_SELECT_SET,
			       start_4d, stride_4d, count_4d, NULL);
    
    dataset = H5Dopen(datasetId,variableName);
    status  = H5Dread(dataset, H5T_NATIVE_FLOAT, memspace, dataspace, 
		      H5P_DEFAULT, unknowns);
    

    int i;
    
    // This is bad why recopy 
    for(i=0;i<sizeofrun*cellDimensions[0]*cellDimensions[1]*cellDimensions[2];i++)
	variable[i] = (float)unknowns[i];


    delete [] unknowns;
    
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);

    return 1;
    
}// End of

int
FlashHDFFile::GetScalarVariable(char variableName[5], int dataPointIndex, int sizeofrun,double bounds[6], double *variable)
{
//    cout << "Loading Variable " << variableName << endl;
    int rank;
    hsize_t dimens_4d[4];
    hid_t ierr;
    
    hid_t dataspace, memspace, dataset;
    
    hsize_t start_4d[5];
    hsize_t stride_4d[5], count_4d[5];
    
    herr_t status;
    
    
    /* ------------------------== unknowns ==--------------------------- */
    if((cellDimensions[0] <= -1) || (cellDimensions[1] <= -1) || 
       (cellDimensions[2] <= -1))
    {
	_SetCellDimensions(); // Cell dimensions have not been set.
    }// End of IF
    	
    rank = 4;
    dimens_4d[0] = numberOfBlocks;    
    dimens_4d[1] = cellDimensions[2]; 
    dimens_4d[2] = cellDimensions[1];
    dimens_4d[3] = cellDimensions[0];

  /* define the dataspace -- as described above */
    start_4d[0]  = (hssize_t)dataPointIndex;
    start_4d[1]  = 0;
    start_4d[2]  = 0;
    start_4d[3]  = 0;
    
    stride_4d[0] = 1;
    stride_4d[1] = 1;
    stride_4d[2] = 1;
    stride_4d[3] = 1;
    
    count_4d[0]  = sizeofrun;// was 1, now the number of blocks grabbed
    count_4d[1]  = cellDimensions[2];
    count_4d[2]  = cellDimensions[1];
    count_4d[3]  = cellDimensions[0];
    
    dataspace = H5Screate_simple(rank, dimens_4d, NULL);

    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
			       start_4d, stride_4d, count_4d, NULL);

    int k3d,k2d;
    k3d = k2d = 1;// Hard coded bad bad bad
    
    /* 
       define the memory space -- the unkt array that was passed includes all
       the variables and guardcells for a single block.  We want to cut out the
       portion that just has the interior cells and the variable specified by 
       index
    */
    rank = 5;
    dimens_4d[0] = numberOfBlocks;
    dimens_4d[1] = cellDimensions[2]+(nguard)*2*k3d;
    dimens_4d[2] = cellDimensions[1]+(nguard)*2*k2d;
    dimens_4d[3] = cellDimensions[0]+(nguard)*2;
    dimens_4d[4] = 1; // Number of Variables 1

    memspace = H5Screate_simple(rank, dimens_4d, NULL);

    /* exclude the guardcells and take only the desired variable */
    start_4d[0] = 0;
    start_4d[1] = (nguard)*k3d;
    start_4d[2] = (nguard)*k2d;  
    start_4d[3] = (nguard);
    start_4d[4] = 0;//variableIndex;  /* should be 0 *//* remember: 0 based indexing */
    
    stride_4d[0] = 1;
    stride_4d[1] = 1;  
    stride_4d[2] = 1;
    stride_4d[3] = 1;
    stride_4d[4] = 1;
    
    count_4d[0] = sizeofrun;
    
    count_4d[1] = cellDimensions[2];
    count_4d[2] = cellDimensions[1]; 
    count_4d[3] = cellDimensions[0]; 
    count_4d[4] = 1; 
    
    // This is bad why hard coded ...
    double *unknowns = new double[sizeofrun*cellDimensions[2]*cellDimensions[1]*cellDimensions[0]];
    
    ierr = H5Sselect_hyperslab(memspace, H5S_SELECT_SET,
			       start_4d, stride_4d, count_4d, NULL);
    
    dataset = H5Dopen(datasetId,variableName);
    status  = H5Dread(dataset, H5T_NATIVE_DOUBLE, memspace, dataspace, 
		      H5P_DEFAULT, unknowns);
    

    int i;
    
    // This is bad why recopy 
    for(i=0;i<sizeofrun*cellDimensions[0]*cellDimensions[1]*cellDimensions[2];i++)
	variable[i] = (double)unknowns[i];


    delete [] unknowns;
    
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);

    // double minbounds[3],maxbounds[3];
    // Get3dMinimumBounds(dataPointIndex, sizeofrun, (double *)&minbounds);
    // Get3dMaximumBounds(dataPointIndex, sizeofrun, (double *)&maxbounds);

    // bounds[0] = (double)minbounds[0];
    // bounds[1] = (double)maxbounds[0];
    // bounds[2] = (double)minbounds[1];
    // bounds[3] = (double)maxbounds[1];    
    // bounds[4] = (double)minbounds[2];
    // bounds[5] = (double)maxbounds[2];    

    return 1;
    
}// End of

int
FlashHDFFile::Get3dMinimumBounds(int idx, double coords[3])
{
    double coord[3];
    double bound[3];
    
    Get3dCoordinate(idx,coord);
    Get3dBlockSize(idx,bound);

    coords[0] = coord[0] - (bound[0]/2.0);
    coords[1] = coord[1] - (bound[1]/2.0);
    coords[2] = coord[2] - (bound[2]/2.0);

	return 1;
}// End of

int
FlashHDFFile::Get3dMaximumBounds(int idx, double coords[3])
{
    double coord[3];
    double bound[3];
    
    Get3dCoordinate(idx,coord);
    Get3dBlockSize(idx,bound);

    coords[0] = coord[0] + (bound[0]/2.0);
    coords[1] = coord[1] + (bound[1]/2.0);
    coords[2] = coord[2] + (bound[2]/2.0);
	return 1;
}// End of

#if 0
int
FlashHDFFile::Get3dMinimumBounds(int idx, int run, double coords[3])
{
    double coord[3];
    double bound[3];
    
    Get3dCoordinate(idx,coord);
    Get3dBlockSize(idx,bound);

    coords[0] = coord[0] - (bound[0]/2.0);
    coords[1] = coord[1] - (bound[1]/2.0);
    coords[2] = coord[2] - (bound[2]/2.0);

	return 1;
}// End of

int
FlashHDFFile::Get3dMaximumBounds(int idx, int run, double coords[3])
{
    double coord[3];
    double bound[3];

    double x,y,z;

    int i;
    for(i=idx;i<(idx+run);i++)
    {
	Get3dCoordinate(idx,coord);
	if(coord[0] >
    Get3dBlockSize(idx,bound);

    coords[0] = coord[0] + (bound[0]/2.0);
    coords[1] = coord[1] + (bound[1]/2.0);
    coords[2] = coord[2] + (bound[2]/2.0);
}// End of
#endif
	
int
FlashHDFFile::GetCoordinateRangeEntireDataset(double ranges[6])
{
    int i;
    double coord[3];
    double bounds[3];
    double emin[3];
    double emax[3];
    
    for(i=0;i<numberOfBlocks;i++)
    {
	Get3dCoordinate(i,coord);
	Get3dBlockSize(i,bounds);

	
	emin[0] = coord[0] - bounds[0]/2.0;
  	emax[0] = coord[0] + bounds[0]/2.0;
	
  	emin[1] = coord[1] - bounds[1]/2.0;
  	emax[1] = coord[1] + bounds[1]/2.0;
	
  	emin[2] = coord[2] - bounds[2]/2.0;
  	emax[2] = coord[2] + bounds[2]/2.0;

	if (i == 0) {
		ranges[0] = emin[0]; ranges[2] = emin[1]; ranges[4] = emin[2];
		ranges[1] = emax[0]; ranges[3] = emax[1]; ranges[5] = emax[2];
	}
	
  	if(emin[0] < ranges[0])
  	    ranges[0] = (double)emin[0];
  	if(emin[1] < ranges[2])
  	    ranges[2] = (double)emin[1];
  	if(emin[2] < ranges[4])
  	    ranges[4] = (double)emin[2];
	
  	if(emax[0] > ranges[1])
  	    ranges[1] = (double)emax[0];
  	if(emax[1] > ranges[3])
  	    ranges[3] = (double)emax[1];
  	if(emax[2] > ranges[5])
  	    ranges[5] = (double)emax[2];
    }// End of FOR

    for(i=0;i<6;i++)
    {
//	cout << "Fh2:" << ranges[i] << " ";
    }
//    cout << endl;
      
    return 1;
      
}// End of GetCoordinateRangeEntireDataset(double range[6])

int
FlashHDFFile::Get3dCoordinate(int idx, double coords[3])
{
    double coord[3];
    int rank;
    hsize_t dimens_1d, dimens_2d[2];
    hid_t ierr;
    
    hid_t dataspace, memspace, dataset;
    
    hsize_t start_2d[2];
    hsize_t stride_2d[2], count_2d[2];
    
    herr_t status;
    rank = 2;
    dimens_2d[0] = numberOfBlocks;
    dimens_2d[1] = 3;//NDIM;
    
    /* define the dataspace -- as described above */
    start_2d[0]  = (hssize_t)idx;
    start_2d[1]  = 0;
    
    stride_2d[0] = 1;
    stride_2d[1] = 1;
    count_2d[0]  = 1;
    count_2d[1]  = 3;//NDIM;
    
    dataspace = H5Screate_simple(rank, dimens_2d, NULL);
    
    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
  			       start_2d, stride_2d, count_2d, NULL);
    
    /* define the memory space */
    rank = 1;
    dimens_1d = 3;//NDIM;
    memspace = H5Screate_simple(rank, &dimens_1d, NULL);
    
    dataset = H5Dopen(datasetId, "coordinates");
    status  = H5Dread(dataset, H5T_NATIVE_DOUBLE, memspace, dataspace, 
  		      H5P_DEFAULT, coord);
    
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);
    
    coords[0] = (double)coord[0];
    coords[1] = (double)coord[1];
    coords[2] = (double)coord[2];
    
    return 1;
    
}// End of 

int
FlashHDFFile::Get3dBlockSize(int idx, double coords[3])
{
    double size[3];
    
    int rank;
    hsize_t dimens_1d, dimens_2d[2];
    hid_t ierr;
    
    hid_t dataspace, memspace, dataset;

    hsize_t start_2d[2];
    hsize_t stride_2d[2], count_2d[2];
    
    herr_t status;
    
    rank = 2;
    dimens_2d[0] = numberOfBlocks;
    dimens_2d[1] = 3;//NDIM;
    
    /* define the dataspace -- as described above */
    start_2d[0]  = (hssize_t)idx;
    start_2d[1]  = 0;
    
    stride_2d[0] = 1;
    stride_2d[1] = 1;
    
    count_2d[0]  = 1;
    count_2d[1]  = 3;//NDIM;
    
    dataspace = H5Screate_simple(rank, dimens_2d, NULL);
    
    ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET,
			       start_2d, stride_2d, count_2d, NULL);
    
    /* define the memory space */
    rank = 1;
    dimens_1d = 3;//NDIM;
    memspace = H5Screate_simple(rank, &dimens_1d, NULL);
    
    dataset = H5Dopen(datasetId, "block size");
    status  = H5Dread(dataset, H5T_NATIVE_DOUBLE, memspace, dataspace, 
  		      H5P_DEFAULT, size);
    
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);
    
    coords[0] = (double)size[0];
    coords[1] = (double)size[1];
    coords[2] = (double)size[2];

	return 1;
    
}// End of Get3dBlockSize(int idx, double coords[3])

int
FlashHDFFile::GetNumberOfVariables()
{
    hid_t dataspace, dataset;

    dataset = H5Dopen(datasetId, "unknown names");
    dataspace = H5Dget_space(dataset);
    
    hsize_t fields[2];
    hsize_t maxfields[2];
    
    H5Sget_simple_extent_dims (dataspace, fields, maxfields);

    H5Sclose(dataspace);
    H5Dclose(dataset);
    
    return fields[0];
    
}// End GetNumberOfVariables()

int 
FlashHDFFile::GetVariableNames(FlashVariableNames **name)
{
    int numberOfVariables = GetNumberOfVariables();
    
    char *tname = new char[4*numberOfVariables]; 

    /* manually set the string size */
    
    hid_t dataset = H5Dopen(datasetId, "unknown names");
    hid_t dataspace = H5Dget_space(dataset);

	 hid_t string_type = H5Dget_type(dataset);
	 
    hid_t status    = H5Dread(dataset, string_type, H5S_ALL, H5S_ALL, 
                       H5P_DEFAULT, tname);

//    cout << tname << endl;
//    cout << strlen(tname) << endl;
    
    *name = new FlashVariableNames[numberOfVariables];

    for(int i=0;i<numberOfVariables;i++)
    {	
	strncpy((*name)[i],&tname[i*4],4);
	(*name)[i][4] = '\0';	
    }
    
    H5Sclose(dataspace);
    H5Dclose(dataset);

    return numberOfVariables;
}

#if 0

int
FlashHDFFile::GetVectorVariable(int variableIndex, int dataPointIndex, double *variable)
{
    int index = SDnametoindex(fileId, "unknowns");
    int id = SDselect(fileId, index);
    
    char sds_name[64];
    int32 rank, dimsizes[MAX_VAR_DIMS], dataType, num_attrs;
	    
    // Retrieves the name, rank, dimension sizes, data type and number of attributes for a data set.
    istat = SDgetinfo(id, sds_name, &rank, dimsizes, &dataType, &num_attrs);
    HDF_ERROR_CHECK(istat,"flashGetCoordinateRangeEntireDataset:1");
    
    numberOfBlocks = dimsizes[0];
    int zRes = dimsizes[1];
    int yRes = dimsizes[2];
    int xRes = dimsizes[3];
    int numberOfVariables = dimsizes[4];
    
#if 0
    cout << "Rank: " << rank << endl;
    cout << "Number of blocks: " << numberOfBlocks << endl;
    cout << "X " << xRes << " Y " << yRes << " Z " << zRes << endl;
    cout << "Number of Variables: " << numberOfVariables << endl;
#endif
    int numberOfDimensions = 0;
    if(xRes > 1)
	numberOfDimensions++;
    if(yRes > 1)
	numberOfDimensions++;
    if(zRes > 1)
	numberOfDimensions++;
    
//    cout << "Number of Dimensions: " << numberOfDimensions << endl;
    
    int k2d = numberOfDimensions/2;
    int k3d = (numberOfDimensions-1)/2;
    
    void *data;
    int cellSizeX = (xRes+nguard-(1+nguard)+1);
    int cellSizeY = (yRes+nguard*k2d-(1+nguard*k2d)+1);
    int cellSizeZ = (zRes+nguard*k3d-(1+nguard*k3d)+1);

//    cout << "Cell X " << cellSizeX << " ";
//    cout << "Cell Y " << cellSizeY << " ";
//    cout << "Cell Z " << cellSizeZ << endl;

    if (dataType == DFNT_DOUBLE32) 
	data = calloc(numberOfVariables * cellSizeX * cellSizeY * cellSizeZ, sizeof(double));
    else if (dataType == DFNT_DOUBLE64) 
	data = calloc(numberOfVariables * cellSizeX * cellSizeY * cellSizeZ, sizeof(double));
    else 
    {
	cout << "ERROR: Unknown datatype" << endl;
	exit(1);
    }// End of ELSE

    int32 start_uk[5], stride_uk[5], edges_uk[5];
    
    start_uk[1] = start_uk[2] = start_uk[3] = start_uk[4] = 0;
    start_uk[0] = dataPointIndex;
    stride_uk[0] = stride_uk[1] = stride_uk[2] = stride_uk[3] = stride_uk[4] = 1;
	
    //	unknowns
    edges_uk[4] = numberOfVariables;
    
    edges_uk[3] = xRes;
    edges_uk[2] = yRes;
    edges_uk[1] = zRes;
    edges_uk[0] = 1;//numberOfBlocks;

    
    istat = SDreaddata(id, (int32 *)start_uk, (int32 *)stride_uk, (int32 *)edges_uk, data);
    HDF_ERROR_CHECK(istat,"Variable Reader");

    int i,j,k,n;
    int internalVariableCount = 0;
    int vectorComponent = 0;
    
    for(k=0; k<cellSizeZ; k++) 
    {
	for(j=0; j<cellSizeY; j++) 
	    for(i=0; i<cellSizeX; i++) 
		for(n=0; n<numberOfVariables; n++) 
		{
		    if((n >= variableIndex) && (n < (variableIndex+numberOfDimensions))) // Variable of Choice
		    {	
			vectorComponent = abs(n-variableIndex);
			
			if(dataType == DFNT_DOUBLE32)
			    variable[(k*cellSizeX*cellSizeY*numberOfDimensions)+(j*cellSizeX*numberOfDimensions)+(i*numberOfDimensions)+vectorComponent]
				= ((double*)data)[internalVariableCount];
			else if(dataType == DFNT_DOUBLE64)
			    variable[(k*cellSizeX*cellSizeY*numberOfDimensions)+(j*cellSizeX*numberOfDimensions)+(i*numberOfDimensions)+vectorComponent]
				= (double)((double*)data)[internalVariableCount];
			else 
			{
			    cout << "datatype_unknown is invalid" << endl;
			    exit(1);
			}// End of ELSE
		    }// End of IF
		    internalVariableCount++;
		}// End of n FOR
    }// End of k FOR
    
    return 1;
    
}// End of


/////////////////
int32 
sdPrintInformation(int32 id)
{
    char sds_name[64];
    int32 rank, dimsizes[MAX_VAR_DIMS], data_type, num_attrs;
    int32 istat;
    
    // Retrieves the name, rank, dimension sizes, data type and number of attributes for a data set.
    istat = SDgetinfo(id, sds_name, &rank, dimsizes, &data_type, &num_attrs);
    HDF_ERROR_CHECK(istat,"sdPrintInformation");

#if PRINT_HDF_INFO
    cout << endl << "sds_name: " << sds_name << endl;
    cout << "rank: " << rank << endl;
    for(int i = 0;i<rank;i++)
        cout << "dimsizes[" << i << "]: " << dimsizes[i] << endl;
    cout << "data_type: " << data_type << endl;
    cout << "num_attrs: " << num_attrs << endl;
#endif

    return data_type;
}// End of sdPrintInformation() 

#endif
