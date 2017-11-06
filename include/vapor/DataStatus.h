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

    vector<string> GetDataMgrNames() const;

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

    //! Return the aggregated time coordinates for all data sets
    //!
    //! This method returns the aggregated time coordinates in
    //! user defined units for all of the currently opened data sets
    //! The time coordinates vector monotonically increasing, and contains
    //! no duplicates.
    //!
    //! \sa GetTimeCoordsFormatted()
    //
    const vector<double> &GetTimeCoordinates() const {
        return (_timeCoords);
    }

    //! Returns a vector of formatted time coordinate strings
    //!
    //! This method interprets the values returned by GetTimeCoordinates()
    //! as seconds since the EPOCH and uses UDUNITS2 to encode the values
    //! the values as year, month, day, hour, minute, second, which are
    //! then formatted as a date-time string.
    //
    const vector<string> &GetTimeCoordsFormatted() const {
        return (_timeCoordsFormatted);
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
    size_t MapGlobalToLocalTimeStep(string dataSetName, size_t ts) const;

    //! Map a local time step to a global time step range
    //!
    //! Map the local time step \p local_ts for the data set named
    //! \p dataSetName to the possible range of global
    //!
    //!
    //! \sa GetTimeCoordinates().
    //
    void MapLocalToGlobalTimeRange(
        string dataSetName, size_t local_ts, size_t &min_ts, size_t &max_ts) const;

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

    //! Determine the minimum time step for which there is any data.
    //! \retval size_t value of smallest time step
    size_t getMinTimestep() { return 0; }

    //! Determine the maximum time step for which there is any data.
    //! \retval size_t value of largest time step
    size_t getMaxTimestep() { return _timeCoords.size() ? _timeCoords.size() - 1 : 0; }

    //! Determine the maximum refinement level present for a variable at a timestep

  private:
    map<string, vector<string>> getFirstVars(
        const vector<string> &dataSetNames) const;

    void reset_time();
    void reset_time_helper();

#ifndef DOXYGEN_SKIP_THIS

    size_t _cacheSize;
    int _nThreads;
    map<string, DataMgr *> _dataMgrs;
    map<string, vector<size_t>> _timeMap;
    vector<double> _timeCoords;
    vector<string> _timeCoordsFormatted;

#endif //DOXYGEN_SKIP_THIS
};

};     // namespace VAPoR
#endif //DATASTATUS_H
