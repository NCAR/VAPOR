//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		DataStatus.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2006
//
//	Description:	Defines the DataStatus class.
//  This class maintains information about the data that is currently
//  loaded.  Maintained and accessed mostly through the Session
#ifndef DATASTATUS_H
#define DATASTATUS_H

#include <vector>
#include <string>
#include <map>
#include <vapor/common.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

//! \class DataStatus
//! \ingroup Public_Params
//! \brief A class for describing the currently loaded dataset
//! \author Alan Norton
//! \version 3.0
//! \date    January 2016
//!
//! The DataStatus class keeps track of available variables, timesteps, resolutions, and data ranges.
//! It is constructed by the Session whenever a new metadata is loaded.
//! It keeps a lazily evaluated value of min/max of each variable for each timestep.
//! Variables can be referenced using the variable name, the session variable num (a numbering all the variables in
//! the session) or by the active variable num.  Active variables are all those in the metadata plus all
//! the derived variables, and are a subset of the session variables.
//! Session variables are those that were specified in the session plus those that are derived, and these may not all be available in the metadata.
//! To support using active variables and session variable nums,
//! mappings are provided between active names/nums and session nums, and also between variable names and
//! their 2D and 3D session variable numbers and active variable numbers.

class ParamsMgr;

class PARAMS_API DataStatus {
  public:
    DataStatus(size_t cacheSize, int nThreads = 0);
    DataStatus() {
        DataStatus(1000);
    }
    virtual ~DataStatus();

    int Open(const std::vector<string> &files, string name, string format);

    void Close(string name);

    //const DataMrgV3_0 *GetDataMgr(string name) const;
    DataMgr *GetDataMgr(string name) const;

    DataMgr *GetDataMgr() const {
        return (GetActiveDataMgr());
    }
    DataMgr *GetActiveDataMgr() const {
        return (GetDataMgr(_activeDataMgr));
    }
    string GetActiveDataMgrName() const {
        return (_activeDataMgr);
    }

    vector<string> GetDataMgrNames() const;

    void SetActiveDataMgr(string name);

    void GetExtents(
        size_t ts,
        const map<string, vector<string>> &varnames,
        vector<double> &minExt, vector<double> &maxExt) const;

    //! Get domain extents for all active variables
    //!
    //! This method returns the union of the domain extents for
    //! all active variables on the window named by \p winName.
    //! A variable is considered active if it
    //! it currrently in use by an enabled RenderParams instance.
    //!
    //! The domain extents returned are always 3D. I.e. \p minExts
    //! and \p maxExts will always have three elements.
    //!
    //! If no variable is active all elements of \p minExts will be zero,
    //! and all elements of maxExts will be one.
    //!
    //! \param[in] paramsMgr Active variables are determined by
    //! querying the ParamsMgr.
    //! \param[out] minExts
    //!
    //! \sa ParamsMgr::GetRenderParams()
    //
    void GetActiveExtents(
        const ParamsMgr *paramsMgr, string winName, size_t ts,
        vector<double> &minExts, vector<double> &maxExts) const;

    void GetActiveExtents(
        const ParamsMgr *paramsMgr, size_t ts,
        vector<double> &minExts, vector<double> &maxExts) const;

    vector<double> GetTimeCoordinates() const {
        return (_timeCoords);
    }

    //! Map a global to a local time step
    //!
    //! Map the global time step \p ts to the closest "local" time step
    //! in the data set named by \p dataSetName. If \p ts is greater
    //! than or equal to GetTimeCoordinates().size then the last time
    //! step in \p dataSetName is returned.
    //!
    //! \return local_ts Returns the local time step, or zero if
    //! \p dataSetName is not recognized.
    //!
    //! \sa GetTimeCoordinates().
    //
    size_t MapTimeStep(string dataSetName, size_t ts) const;

    //! Set number of execution threads
    //!
    //! Set the number of execution threads. If \p nThreads == 0, the
    //! default,
    //! the system will attempt to set the number of threads equal to
    //! the number of cores detected. Has no effect until
    //! the next data set is loaded.
    //
    void SetNumThreads(size_t nthreads) {
        _nThreads = nthreads;
    }

    size_t GetNumThreads() const {
        return _nThreads;
    }

    //! Set the data cache size
    //!
    //! Set the size of the data cache in MBs.
    //! Has no effect until
    //! the next data set is loaded.
    //!
    //! \sa DataMgr
    //
    void SetCacheSize(size_t sizeMB) {
        _cacheSize = sizeMB;
    }

    //! Obtain the full extents of the current data in local coordinates.
    //! Values in this array are in the order: minx, miny, minz, maxx, maxy, maxz.
    //! By definition, the first three are zero.
    //! \retval const double[6] extents array
    const double *getLocalExtents() const { return _extents; }

    void GetExtents(vector<double> &minExt, vector<double> &maxExt) const {
        minExt.clear(), maxExt.clear();
        for (int i = 0; i < 3; i++) {
            minExt.push_back(_extents[i]);
            maxExt.push_back(_extents[i + 3]);
        }
    }

    //! Obtain the full size of the current data in user coordinates.
    //! Values in this array are in the order: sizex,sizey,sizez.
    //! \retval const double[3] sizes array
    const double *getFullSizes() { return _fullSizes; }

    //! Obtain the full size of the current data in stretched user coordinates.
    //! Values in this array are in the order: sizex,sizey,sizez.
    //! \retval const double[3] sizes array
    const double *getFullStretchedSizes() { return _fullStretchedSizes; }

    //! Obtain the full extents of the current data in stretched local coordinates.
    //! Values in this array are in the order: minx, miny, minz, maxx, maxy, maxz.
    //! \retval const double[6] extents array
    const double *getStretchedLocalExtents() { return _stretchedExtents; }

    //! Determine the minimum time step for which there is any data.
    //! \retval size_t value of smallest time step
    size_t getMinTimestep() { return _minTimeStep; }

    //! Determine the maximum time step for which there is any data.
    //! \retval size_t value of largest time step
    size_t getMaxTimestep() { return _maxTimeStep; }

    //! Indicates the number of time steps in the current VDC.
    //! \return integer number of timesteps
    int getNumTimesteps() { return _numTimesteps; }

#ifdef DEAD
    //! Determine the index of a specified variable name
    //! \param[in] dimension of variable (2 or 3)
    //! \param[in] varname name of variable
    //! \return index of specified variable or -1 if it does not exist.
    int getActiveVarIndex(int dim, string varname);
#endif

    //! Determine the size of a voxel in user coordinates, along a specific dimension,
    //! or its maximum or minimum dimension
    //! \param[in] timestep of variable used to determine dimensions
    //! \param[in] varname Variable name used for determining dimension.
    //! \param[in] reflevel Refinement level at which voxel is measured, or -1 for maximum ref level
    //! \param[in] dir is 0,1,2 for x,y,z dimension.  Dir is -1 for maximum, -2 for minimum.
    double getVoxelSize(size_t timestep, string varname, int refLevel, int dir);

#ifdef DEAD
    //! Determine the number of active 3D variables.  This includes 3D variables in the VDC as well as any 3D derived variables
    //! \return number of active 3D variables
    int getNumActiveVariables3D() { return _dataMgr->GetDataVarNames(3, true).size(); }
    //! Determine the number of active 2D variables.  This includes 2D variables in the VDC as well as any 2D derived variables
    //! \return number of active 2D variables
    int getNumActiveVariables2D() { return _dataMgr->GetDataVarNames(2, true).size(); }
    //! Determine the number of active variables.  This includes variables in the VDC as well as any derived variables
    //! \return number of active variables
    int getNumActiveVariables() { return getNumActiveVariables3D() + getNumActiveVariables2D(); }

#endif

#ifdef DEAD
    //! Obtain default variable range lazily
    //! Saves bounds in MapperFunction if values not previously initialized
    //! Returns true if range is being set for the first time
    //! This should be called before the MapperFunction bounds are displayed,
    //! as it will initialize them to default values if they have not yet been set.
    //! \param[in] varname Variable Name to use
    //! \param[in] mf Mapper Function that holds the current bounds. If null, is ignored.
    //! \param[out] minmax Resulting bounds.
    //! \return true if this is the first time the bounds of this variable have been obtained
    bool GetDefaultVariableRange(string varname, MapperFunction *mf, float minmax[2]);

#endif

    //! Convert user point coordinates to lon/lat in-place.  Return false if can't do it.
    //! \param[in/out] coords coordinates to be converted
    //! \param[in] npoints Number of points to convert.
    //! \return true if successful
    bool convertToLonLat(double coords[], int npoints = 1);
    //! Convert lon/lat coordinates to user coordinates in-place.  Return false if can't do it.
    //! \param[in/out] coords coordinates to be converted
    //! \param[in] npoints Number of points to convert.
    //! \return true if successful
    bool convertFromLonLat(double coords[], int npoints = 1);
    //! Convert local user point coordinates to lon/lat in-place.  Return false if can't do it.
    //! If the timestep is negative, then the coords use non-time-varying extents.
    //! \param[in] timestep  Time step to use for conversion
    //! \param[in/out] coords coordinates to be converted
    //! \param[in] npoints Number of points to convert.
    //! \return true if successful
    bool convertLocalToLonLat(int timestep, double coords[], int npoints = 1);
    //! Map Longitude/latitude to local user coordinates
    //! If the timestep is negative, then the coords use non-time-varying extents.
    //! \param[in] timestep  Time step to use for conversion
    //! \param[in/out] coords coordinates to be converted
    //! \param[in] npoints Number of points to convert.
    //! \return true if successful
    bool convertLocalFromLonLat(int timestep, double coords[], int npoints = 1);

#ifdef DEAD
    //! Map corners of box to voxel coordinates.
    //! Result is zero if box does not lie in data domain.
    //! \param[in] box is Box to be mapped
    //! \param[in] varname Name of variable whose coordinates to use
    //! \param[in] refLevel is refinement level to be used in the mapping
    //! \param[in] lod is LOD level to be used for the mapping
    //! \param[in] timestep is time step to use in the mapping
    //! \param[out] voxExts Voxel extents of box, zeroes if not in data bounds
    void mapBoxToVox(
        Box *box, string varname, int refLevel, int lod,
        int timestep, size_t voxExts[6]);
#endif

    //! Obtain the time-varying extents, based on the domain-defining variables.
    //! Return true on success.
    bool GetExtents(
        size_t timestep, vector<double> &minExts, vector<double> &maxExts);

#ifdef DEAD
    //! Return the current scene stretch factors
    //! \retval const float* current stretch factors
    const vector<double> getStretchFactors() {
        return ((VizFeatureParams *)_paramsMgr->GetDefaultParams(Params::_visualizerFeaturesParamsTag))->GetStretchFactors();
    }
#endif
    vector<double> getStretchFactors() const {
        return (_stretchFactors);
    }

    void setStretchFactors(vector<double> val) {
        assert(val.size() == 3);
        _stretchFactors = val;
    }

    //! Stretch a 3-vector based on current stretch factors
    //! \param[in/out] vector<double> coords[3]
    void stretchCoords(vector<double> &coords) {
        vector<double> stretchFactors = getStretchFactors();
        for (int i = 0; i < 3; i++)
            coords[i] = coords[i] * stretchFactors[i];
    }
    //! Stretch a 3-vector based on current stretch factor
    //! \param[in/out] float coords[3]
    void stretchCoords(double coords[3]) {
        vector<double> stretchFactors = getStretchFactors();
        for (int i = 0; i < 3; i++)
            coords[i] = coords[i] * stretchFactors[i];
    }
    //! Find the max domain extent in stretched coords
    //! \retval float maximum stretched extent
    double getMaxStretchedSize() {
        return (max(_fullStretchedSizes[0], max(_fullStretchedSizes[1], _fullStretchedSizes[2])));
    }
    //! Convert local user coordinates to stretched local user coordinates
    //! \param[in] fromCoords local user coordinates
    //! \param[out] toCoords stretched local user coordinates
    void localToStretched(const double fromCoords[3], double toCoords[3]);
    //! Determine the maximum refinement level present for a variable at a timestep
    //! \param[in] varname variable name
    //! \param[in] timestep Time Step
    //! \return maximum refinement level (transform) present
    int maxXFormPresent(string varname, size_t timestep) const;
    //! Determine the maximum compression level present for a variable at a timestep
    //! \param[in] varname variable name
    //! \param[in] timestep Time Step
    //! \return maximum compression level (LOD) present
    int maxLODPresent(string varname, size_t timestep) const;
    //! Method indicates if user allows using lower accuracy, when specified LOD or refinement is not available.
    //! \retval bool true if lower accuracy is requested.
    bool useLowerAccuracy() const { return _useLowerAccuracy; }
    //! Turn on or off the option of using lower accuracy when requested accuracy is not available
    //! \param[in] val True to turn on this option
    void setUseLowerAccuracy(bool val) { _useLowerAccuracy = val; }
    //! Determine if any data is present at a timestep
    //! \param[in] timestep Time Step to be checked
    //! \return true if any variable is present at the time step
    bool dataIsPresent(int timestep);
    //! Perform a mapping of Voxel coordinates to user coordinates
    //! based on a specified variable
    //! \param[in] timestep  time step used in mapping
    //! \param[in] varname variable used in mapping
    //! \param[in] vCoords voxel coordinates
    //! \param[out] uCoords user coordinates
    //! \param[in] reflevel Refinement level used in the mapping
    int mapVoxToUser(size_t timestep, string varname, const size_t vcoords[3], double uCoords[3], int reflevel);
    //! Perform a mapping of User coordinates to Voxel coordinates
    //! based on a specified variable
    //! \param[in] timestep  time step used in mapping
    //! \param[in] varname variable used in mapping
    //! \param[in] uCoords User coordinates
    //! \param[out] vCoords voxel coordinates
    //! \param[in] reflevel Refinement level used in the mapping
    void mapUserToVox(size_t timestep, string varname, const double uCoords[3], size_t vCoords[3], int reflevel);

    //! Update DataStatus based on a stretch factor.
    //! This must be called whenver the stretch factor changes.
    //! \param factor 3-vector of new stretch factors
    void stretchExtents(vector<double> factor) {
        for (int i = 0; i < 3; i++) {
            _stretchedExtents[i] = _extents[i] * factor[i];
            _stretchedExtents[i + 3] = _extents[i + 3] * factor[i];
            _fullStretchedSizes[i] = _fullSizes[i] * factor[i];
        }
    }

    //! Identify the dimension associated with a data variable name.
    //! \param[in] varname Variable name
    //! \return dimension (2 or 3) or -1 if variable or DataMgr is invalid.
    int GetVariableDimension(string varname) {
        DataMgr *dataMgr = GetActiveDataMgr();
        if (!dataMgr)
            return (2);
        vector<size_t> dims;
        vector<size_t> bs;
        int rc = dataMgr->GetDimLensAtLevel(varname, 0, dims, bs);
        if (rc)
            return rc;
        return dims.size();
    }

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
    //! \param[in/out] *lod : requested lod may be reduced if only
    //! a lower lod is available.
    //! \param[out] grid is an array of StructuredGrid* pointers, one
    //! for each variable
    //! \return zero if successful.
    int getGrids(
        size_t ts,
        const vector<string> &varnames,
        const vector<double> &minExtsReq, const vector<double> &maxExtsReq,
        int *refLevel,
        int *lod,
        StructuredGrid **grids) const;

    int getGrids(
        size_t ts, const vector<string> &varnames,
        int *refLevel, int *lod, StructuredGrid **grids) const;

    //! \deprecated
    //
    int getGrids(
        size_t ts,
        const vector<string> &varnames,
        const double extents[6],
        int *refLevel,
        int *lod,
        StructuredGrid **grids) const;

  private:
    map<string, vector<string>> getFirstVars(
        const vector<string> &dataSetNames) const;

    //! Reset the datastatus when a new datamgr is opened.
    //! This must be called whenever the data manager changes or when any new variables are defined.
    void reset();
    void reset_time();

#ifndef DOXYGEN_SKIP_THIS

    size_t _cacheSize;
    int _nThreads;
    map<string, DataMgr *> _dataMgrs;
    string _activeDataMgr;
    map<string, vector<size_t>> _timeMap;
    vector<double> _timeCoords;

    //specify the minimum and max time step that actually have data:
    size_t _minTimeStep;
    size_t _maxTimeStep;

    //numTimeSteps may include lots of times that are not used.
    int _numTimesteps;

    double _extents[6];
    double _stretchedExtents[6];
    double _fullSizes[3];
    double _fullStretchedSizes[3];
    vector<double> _stretchFactors;
    bool _useLowerAccuracy;
    vector<string> _domainVars;

#endif //DOXYGEN_SKIP_THIS
};

};     // namespace VAPoR
#endif //DATASTATUS_H
