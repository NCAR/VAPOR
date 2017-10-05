//************************************************************************
//									*	
//		     Copyright (C)  2017				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		DataMgrUtils.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2017
//
//	Description:	Defines the DataMgrUtils free functions.  
//
//  These  functions operate on instances of the DataMgr class.
//
#ifndef DATAMGRUTILS_H
#define DATAMGRUTILS_H


#include <vector>
#include <string>
#include <map>
#include <vapor/DataMgr.h>

namespace VAPoR {
namespace DataMgrUtils {

 //! Return the maximum available transform level for a variable.
 //!
 //! This method checks for the existence of a variable at all 
 //! available transform levels (see DataMgr::GetNumRefLevels()), and 
 //! returns the highest available level. The minimum level is zero
 //! and the max is DataMgr::GetNumRefLevels() - 1.
 //!
 //! \param[in] timestep Time Step
 //! \param[in] varname variable name
 //! \param[out] maxXForm Maximum available level
 //!
 //! \return status Return true on success. Return false if the no
 //! transform levels exist e.g. the variable does not exist.
 //
 bool MaxXFormPresent(
	 const DataMgr *dataMgr, size_t timestep, string varname, size_t &maxXForm
 );

 //! Return the maximum available LOD level for a variable.
 //!
 //! This method checks for the existence of a variable at all 
 //! available levels of detail (see DataMgr::GetCRatios()), and 
 //! returns the highest available level. The minimum level is zero
 //! and the max is DataMgr::GetCRatios().size() - 1.
 //!
 //! \param[in] timestep Time Step
 //! \param[in] varname variable name
 //! \param[out] maxLOD Maximum available level
 //!
 //! \return status Return true on success. Return false if the no
 //! LOD levels exist e.g. the variable does not exist.
 //
 bool MaxLODPresent( 
     const DataMgr *dataMgr, size_t timestep, string varname, size_t &maxLOD
 );

 //! Convert Projected Coordinate System coordinates to lon/lat in-place. 
 //!
 //! Perform in-place conversion of an array of interleaved pairs of 
 //! coordinates from PCS
 //! to lon-lat coordinates. The input pairs are ordered X, then Y. The output
 //! pairs are ordered Longitude, then Latitude
 //! Return false if can't do it.
 //!
 //! \param[in/out] coords coordinates to be converted
 //! \param[in] npoints Number of points to convert.
 //! \return true if successful
 //!
 int ConvertPCSToLonLat(
	const DataMgr *dataMgr,
	double coords[], int npoints = 1 
 );

 //! Convert lon/lat coordinates to Projected Coordinate System coordinates,
 //!  in-place. 
 //!
 //! Perform in-place conversion of an array of interleaved pairs of 
 //! coordinates from lon-lat 
 //! to PCS coordinates. The input pairs are ordered longitude, then 
 //! latitude. The output
 //! pairs are ordered X, then Y
 //! Return false if can't do it.
 //!
 //! \param[in/out] coords coordinates to be converted
 //! \param[in] npoints Number of points to convert.
 //! \return true if successful
 //!
 int ConvertLonLatToPCS(
	const DataMgr *dataMgr,
	double coords[], int npoints = 1
 );

 //! Method that obtains one or more regular grids at specified timestep, 
 //! extents, refinement, and lod.
 //! If the data is available, but not at the requested extents, 
 //! refinement or lod, then the extents may
 //! be reduced, and the data accuracy may be reduced.
 //!
 //! All variables must have same spatial dimensionality
 //!
 //! \param[in] ts timestep being requested
 //! \param[in] variable name(s) being requested
 //! \param[in] minExtsReq Minimum requested extents
 //! \param[in] maxExtsReq Maximum requested extents
 //! \param[in/out] *refLevel : requested refinement may be 
 //! reduced if only a lower level is available
 //! \param[in] useLowerAccuracy If true use lower accuracy data then
 //! requested if the requested accuracy is not available
 //! \param[in/out] *lod : requested lod may be reduced if only 
 //! a lower lod is available.
 //! \param[out] grid is a vector of Grid* pointers, one 
 //! for each variable
 //
 VDF_API int GetGrids(
	DataMgr *dataMgr,
	size_t ts, 
	const vector<string>& varnames, 
	const vector <double> &minExtsReq, const vector <double> &maxExtsReq,
	bool useLowerAccuracy,
	int* refLevel, 
	int* lod, 
	std::vector <Grid *> &grids
 );

 VDF_API int GetGrids(
	DataMgr *dataMgr, size_t ts, string varname, 
	const vector <double> &minExtsReq, const vector <double> &maxExtsReq,
	bool useLowerAccuracy, int* refLevel, int* lod, Grid ** gridptr
 );

 VDF_API int GetGrids(
	DataMgr *dataMgr,
	size_t ts, const vector<string>& varnames,
	bool useLowerAccuracy, int* refLevel, int* lod, 
	std::vector <Grid *> &grids
 );

 VDF_API int GetGrids(
	DataMgr *dataMgr, size_t ts, string varname, 
	bool useLowerAccuracy, int* refLevel, int* lod, Grid ** gridptr
 );

 //! Get the spatial coordinate axes for a variable
 //!
 //! Returns the ordered (fastest to slowest varying) coordinate axis
 //! for a named variable. 
 //!
 //! \param[in] varname Name of variable
 //! \param[out] axes Ordered list of axis indecies. The range of possible
 //! axes values is [0..2], with 0 corresponding to the X coordinate axis,
 //! 1 corresponding to Y, and 2 to Z.
 //!
 bool GetAxes(
	const DataMgr *dataMgr, string varname, vector <int> &axes
 );

 //! Get a variables coordinate extents
 //!
 //! Get the minimum and maximum coordinate extents of a name variable
 //! at a given time step. The extents are returned in \p minExts and 
 //! \p maxExts. The size of these vectors will match the dimensionality 
 //! of the variable. The GetAxes() method can be used to determine which
 //! coordinate axes the returned extents correspond to.
 //!
 //! If \p varname is an empty string the function scans the list of 
 //! available data variables looking for the highest dimension variable
 //! available.
 //!
 //! \param[in] timestep Time step of variable. Ignored for variables that
 //! are not time-varying.
 //! \param[in] Name of variable
 //! \param[out] minExts A vector whose size matches the dimensionality
 //! of the variable, and containing the minimum ordered coordinate
 //! extents of \p varname at time step \p timestep.
 //! \param[out] maxExts A vector whose size matches the dimensionality
 //! of the variable, and containing the maximum ordered coordinate
 //! extents of \p varname at time step \p timestep.
 //!
 //! \sa GetAxes()
 //
 VDF_API bool GetExtents(
	DataMgr *dataMgr,
	size_t timestep, string varname,
	vector <double>& minExts, vector <double>& maxExts
 );

 //! Get coordinate extents for one or more variables.
 //!
 //! Get the minimum and maximum coordinate extents of a list of variables
 //! at a given time step. This function handles variables with mixed
 //! dimensionality, and 2D variables that are defined on different planes.
 //! The extents are returned in \p minExts and 
 //! \p maxExts. 
 //!
 //! \param[in] timestep Time step of variable. Ignored for variables that
 //! are not time-varying.
 //! \param[in] Name of variable
 //! \param[out] minExts A vector whose size matches the dimensionality
 //! of the variable, and containing the minimum ordered coordinate
 //! extents of \p varname at time step \p timestep.
 //! \param[out] maxExts A vector whose size matches the dimensionality
 //! of the variable, and containing the maximum ordered coordinate
 //! extents of \p varname at time step \p timestep.
 //!
 //! \param[out] axes A vector indicating the axis of each coordinate 
 //! returned in \p minExts and \p maxExts. See GetAxes()
 //!
 //! \sa GetAxes()
 //
 VDF_API bool GetExtents(
	DataMgr *dataMgr,
	size_t timestep, const vector <string> &varnames,
	vector <double>& minExts, vector <double>& maxExts,
	vector <int> &axes
 );

#ifdef	DEAD



 //! Determine the size of a voxel in user coordinates, along a specific dimension, 
 //! or its maximum or minimum dimension
 //! \param[in] timestep of variable used to determine dimensions
 //! \param[in] varname Variable name used for determining dimension.
 //! \param[in] reflevel Refinement level at which voxel is measured, or -1 for maximum ref level
 //! \param[in] dir is 0,1,2 for x,y,z dimension.  Dir is -1 for maximum, -2 for minimum.
 double getVoxelSize(size_t timestep, string varname, int refLevel, int dir);
 
//! Determine the number of active 3D variables.  This includes 3D variables in the VDC as well as any 3D derived variables
//! \return number of active 3D variables
int getNumActiveVariables3D() {return _dataMgr->GetDataVarNames(3,true).size();}
//! Determine the number of active 2D variables.  This includes 2D variables in the VDC as well as any 2D derived variables
//! \return number of active 2D variables
int getNumActiveVariables2D() {return _dataMgr->GetDataVarNames(2,true).size();}
//! Determine the number of active variables.  This includes variables in the VDC as well as any derived variables
//! \return number of active variables
int getNumActiveVariables() {return getNumActiveVariables3D()+getNumActiveVariables2D();}



 //! Map corners of box to voxel coordinates.  
 //! Result is zero if box does not lie in data domain.
 //! \param[in] box is Box to be mapped
 //! \param[in] varname Name of variable whose coordinates to use
 //! \param[in] refLevel is refinement level to be used in the mapping
 //! \param[in] lod is LOD level to be used for the mapping
 //! \param[in] timestep is time step to use in the mapping
 //! \param[out] voxExts Voxel extents of box, zeroes if not in data bounds
 void mapBoxToVox(
	Box* box, string varname, int refLevel, int lod, 
	int timestep, size_t voxExts[6]
 );


#endif

}; //end VAPoR namespace
};
#endif // DATAMGRUTILS_H

