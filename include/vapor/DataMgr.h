#include <vector>
#include <iostream>
#include <list>
#include <cassert>
#include <vapor/BlkMemMgr.h>
#include <vapor/DC.h>
#include <vapor/MyBase.h>
#include <vapor/RegularGrid.h>
#include <vapor/StretchedGrid.h>
#include <vapor/LayeredGrid.h>
#include <vapor/CurvilinearGrid.h>
#include <vapor/UnstructuredGrid2D.h>
#include <vapor/KDTreeRG.h>

#ifndef DataMgvV3_0_h
    #define DataMgvV3_0_h

using namespace std;

namespace VAPoR {
class PipeLine;

//! \class DataMgr
//! \brief A cache based data reader
//! \author John Clyne
//!
//! The DataMgr class is an abstract class that defines public methods for
//! accessing (reading) 1D, 2D and 3D field variables. The class implements a
//! memory cache to speed data access -- once a variable is read it is
//! stored in cache for subsequent access. The DataMgr class is abstract:
//! it declares a number of public and protected pure virtual methods
//! that must be
//! implemented by specializations of this class to allow access to particular
//! file formats.
//!
//! This class inherits from Wasp::MyBase. Unless otherwise documented
//! any method that returns an integer value is returning status. A negative
//! value indicates failure. Error messages are logged via
//! Wasp::MyBase::SetErrMsg().
//!
//! Methods that return a boolean do
//! not, unless otherwise documented, log an error message upon
//! failure (return of false).
//!
//! \param level
//! \parblock
//! Grid refinement level for multiresolution variables.
//! Compressed variables in the VDC have a multi-resolution
//! representation: the sampling grid for multi-resolution variables
//! is hierarchical, and the dimension lengths of adjacent levels in the
//! hierarchy differ by a factor of two. The \p level parameter is
//! used to select a particular depth of the hierarchy.
//!
//! To provide maximum flexibility as well as compatibility with previous
//! versions of the VDC the interpretation of \p level is somewhat
//! complex. Both positive and negative values may be used to specify
//! the refinement level and have different interpretations.
//!
//! For positive
//! values of \p level, a value of \b 0 indicates the coarsest
//! member of the
//! grid hierarchy. A value of \b 1 indicates the next grid refinement
//! after the coarsest, and so on. Using postive values the finest level
//! in the hierarchy is given by GetNumRefLevels() - 1. Values of \p level
//! that are greater than GetNumRefLevels() - 1 are treated as if they
//! were equal to GetNumRefLevels() - 1.
//!
//! For negative values of \p level a value of -1 indicates the
//! variable's native grid resolution (the finest resolution available).
//! A value of -2 indicates the next coarsest member in the hierarchy after
//! the finest, and so
//! on. Using negative values the coarsest available level in the hierarchy is
//! given by negating the value returned by GetNumRefLevels(). Values of
//! \p level that are less than the negation of GetNumRefLevels() are
//! treated as if they were equal to the negation of the GetNumRefLevels()
//! return value.
//! \endparblock
//!
//! \param lod
//! \parblock
//! The level-of-detail parameter, \p lod, selects
//! the approximation level for a compressed variable.
//! The \p lod parameter is similar to the \p level parameter in that it
//! provides control over accuracy of a compressed variable. However, instead
//! of selecting the grid resolution the \p lod parameter controls
//! the compression factor by indexing into the \p cratios vector (see below).
//! As with the \p level parameter, both positive and negative values may be
//! used to index into \p cratios and
//! different interpretations.
//!
//! For positive
//! values of \p lod, a value of \b 0 indicates the
//! the first element of \p cratios, a value of \b 1 indicates
//! the second element, and so on up to the size of the
//! \p cratios vector (See DC::GetCRatios()).
//!
//! For negative values of \p lod a value of \b -1 indexes the
//! last element of \p cratios, a value of \b -2 indexes the
//! second to last element, and so on.
//! Using negative values the first element of \p cratios - the greatest
//! compression rate - is indexed by negating the size of the
//! \p cratios vector.
//! \endparblock
//
class VDF_API DataMgr : public Wasp::MyBase {
public:
    //! Constructor for the DataMgr class.
    //!
    //! The DataMgr will attempt to cache previously read data and coordinate
    //! variables in memory. The \p mem_size specifies the requested cache
    //! size in MEGABYTES!!!
    //!
    //! \param[in] format A string indicating the format of data collection.
    //!
    //! \param[in] mem_size Size of memory cache to be created, specified
    //! in MEGABYTES!!
    //!
    //! \param[in] numthreads Number of parallel execution threads
    //! to be run during encoding and decoding of compressed data. A value
    //! of 0, the default, indicates that the thread count should be
    //! determined by the environment in a platform-specific manner, for
    //! example using sysconf(_SC_NPROCESSORS_ONLN) under *nix OSes.
    //!
    //
    DataMgr(string format, size_t mem_size, int nthreads = 0);

    virtual ~DataMgr();

    //! Initialize the class
    //!
    //! This method must be called to initialize the DataMgr class with
    //! a list of input data files.
    //!
    //! \param[in] files A list of file paths
    //!
    //! \retval status A negative int is returned on failure and an error
    //! message will be logged with MyBase::SetErrMsg()
    //!
    //
    virtual int Initialize(const std::vector<string> &files);

    //! \copydoc DC::GetDimensionNames()
    //
    std::vector<string> GetDimensionNames() const
    {
        assert(_dc);
        return (_dc->GetDimensionNames());
    }

    //! \copydoc DC::GetDimension()
    //
    bool GetDimension(string dimname, DC::Dimension &dimension) const
    {
        assert(_dc);
        return (_dc->GetDimension(dimname, dimension));
    }

    //! \copydoc DC::GetMeshNames()
    //
    std::vector<string> GetMeshNames() const
    {
        assert(_dc);
        return (_dc->GetMeshNames());
    }

    //! \copydoc DC::GetMesh()
    //
    bool GetMesh(string meshname, DC::Mesh &mesh) const
    {
        assert(_dc);
        return (_dc->GetMesh(meshname, mesh));
    }

    //! Return a list of names for all of the defined data variables.
    //!
    //! This method returns a list of all data variables defined
    //! in the data set.
    //!
    //! \retval list A vector containing a list of all the data variable names
    //!
    //! \sa GetCoordVarNames()
    //!
    //! \test New in 3.0
    virtual std::vector<string> GetDataVarNames() const;

    //! Return a list of data variables with a given dimension rank
    //!
    //! This method returns a list of all data variables defined
    //! in the data set with the specified dimension rank (number of dimensions).
    //! Data variables may have 0 to 3 spatial dimensions, and 0 of 1 time
    //! dimension.
    //!
    //! \param[in] ndim Variable rank (number of dimensions)
    //! \param[in] spatial A boolean, if true, indicates that only the variable's
    //! spatial rank should be compared against \p ndim.  Otherwise the sum
    //! of the number of spatial dimensions plus the time dimension (if time
    //! varying) is compared agains \p ndim.
    //!
    //! \retval list A vector containing a list of all the data variable names
    //! with the specified number of dimensions (rank).
    //!
    //! \test New in 3.0
    virtual std::vector<string> GetDataVarNames(int ndim, bool spatial) const;

    //! Return a list of names for all of the defined coordinate variables.
    //!
    //! This method returns a list of all coordinate variables defined
    //! in the data set.
    //!
    //! \retval list A vector containing a list of all the coordinate
    //! variable names
    //!
    //! \sa GetDataVarNames()
    //!
    virtual std::vector<string> GetCoordVarNames() const;

    //! Return a list of coordinate variables with a given dimension rank
    //!
    //! This method returns a list of all coordinate variables defined
    //! in the coordinate set with the specified dimensionality
    //!
    //! \param[in] ndim Variable rank
    //! \param[in] spatial A boolean indicating whether only the variables
    //! spatial rank should be compared against \p ndim
    //!
    //! \retval list A vector containing a list of all the coordinate
    //! variable names
    //! with the specified number of dimensions.
    //!
    virtual std::vector<string> GetCoordVarNames(int ndim, bool spatial) const;

    //! Get time coordinates
    //!
    //! Get a sorted list of all time coordinates defined for the
    //! data set. Multiple time coordinate variables may be defined. This method
    //! collects the time coordinates from all time coordinate variables
    //! and sorts them into a single, global time coordinate variable.
    //!
    //! \note Need to deal with different units (e.g. seconds and days).
    //! \note Should methods that take a time step argument \p ts expect
    //! "local" or "global" time steps
    //!
    void                GetTimeCoordinates(std::vector<double> &timecoords) const { timecoords = _timeCoordinates; };
    std::vector<double> GetTimeCoordinates() const { return (_timeCoordinates); };

    //! Get time coordinate var name
    //!
    //! Return the name of the time coordinate variable. If no time coordinate
    //! variable is defined the empty string is returned.
    //
    string GetTimeCoordVarName() const;

    //! Return an ordered list of a data variable's coordinate names
    //!
    //! Returns a list of a coordinate variable names for the variable
    //! \p varname, ordered from fastest
    //! to slowest. If \p spatial is true and the variable is time varying
    //! the time coordinate variable name will be included. The time
    //! coordinate variable is always
    //! the slowest varying coordinate axis
    //!
    //! \param[in] varname A valid variable name
    //! \param[in] spatial If true only return spatial dimensions
    //!
    //! \param[out] coordvars Ordered list of coordinate variable names.
    //!
    //! \retval Returns true upon success, false if the variable is
    //! not defined.
    //!
    //
    virtual bool GetVarCoordVars(string varname, bool spatial, std::vector<string> &coord_vars) const;

    //! Return a data variable's definition
    //!
    //! Return a reference to a DC::DataVar object describing
    //! the data variable named by \p varname
    //!
    //! \param[in] varname A string specifying the name of the variable.
    //! \param[out] datavar A DataVar object containing the definition
    //! of the named Data variable.
    //!
    //! \retval bool If true the method was successful. If false the
    //! named variable is invalid (unknown)
    //!
    //! \sa GetCoordVarInfo()
    //!
    bool GetDataVarInfo(string varname, VAPoR::DC::DataVar &datavar) const;

    //! Return metadata about a data or coordinate variable
    //!
    //! If the variable \p varname is defined as either a
    //! data or coordinate variable its metadata will
    //! be returned in \p var.
    //!
    //! \retval bool If true the method was successful. If false the
    //! named variable is invalid (unknown)
    //!
    //! \sa GetDataVarInfo(), GetCoordVarInfo()
    //
    bool GetBaseVarInfo(string varname, VAPoR::DC::BaseVar &var) const;

    //! Return a coordinate variable's definition
    //!
    //! Return a reference to a DC::CoordVar object describing
    //! the coordinate variable named by \p varname
    //!
    //! \param[in] varname A string specifying the name of the coordinate
    //! variable.
    //! \param[out] coordvar A CoordVar object containing the definition
    //! of the named variable.
    //!
    //! \retval bool If true the method was successful. If false the
    //! named variable is invalid (unknown)
    //!
    //! \sa GetDataVarInfo()
    //!
    bool GetCoordVarInfo(string varname, VAPoR::DC::CoordVar &cvar) const;

    //! Return a boolean indicating whether a variable is time varying
    //!
    //! This method returns \b true if the variable named by \p varname is defined
    //! and it has a time axis dimension. If either of these conditions
    //! is not true the method returns false.
    //!
    //! \param[in] varname A string specifying the name of the variable.
    //! \retval bool Returns true if variable \p varname exists and is
    //! time varying.
    //!
    bool IsTimeVarying(string varname) const;

    //! Return a boolean indicating whether a variable is compressed
    //!
    //! This method returns \b true if the variable named by \p varname is defined
    //! and it has a compressed representation. If either of these conditions
    //! is not true the method returns false.
    //!
    //! \param[in] varname A string specifying the name of the variable.
    //! \retval bool Returns true if variable \p varname exists and is
    //! compressed
    //!
    //! \sa DefineCoordVar(), DefineDataVar(), DC::BaseVar::GetCompressed()
    //
    bool IsCompressed(string varname) const;

    //! Return the time dimension length for a variable
    //!
    //! Returns the number of time steps (length of the time dimension)
    //! for which a variable is defined. If \p varname does not have a
    //! time coordinate 1 is returned. If \p varname is not defined
    //! as a variable a negative int is returned.
    //!
    //! \param[in] varname A string specifying the name of the variable.
    //! \retval count The length of the time dimension, or a negative
    //! int if \p varname is undefined.
    //!
    //! \sa IsTimeVarying()
    //
    int GetNumTimeSteps(string varname) const;

    //! \copydoc DC::GetNumRefLevels()
    //
    size_t GetNumRefLevels(string varname) const;

    //! \copydoc DC::GetCRatios()
    //
    std::vector<size_t> GetCRatios(string varname) const;

    //! Read and return variable data
    //!
    //! Reads all data for the data or coordinate variable named by \p varname
    //! for the time step, refinement level, and leve-of-detail indicated
    //! by \p ts, \p level, and \p lod, respectively.
    //!
    //! \param[in] ts
    //! An integer offset into the time coordinate variable
    //! returned by GetTimeCoordinates() indicting the time step of the
    //! variable to access.
    //!
    //! \param[in] varname The name of the data or coordinate variable to access
    //!
    //! \param[in] level Grid refinement level. See DataMgr
    //!
    //! \param[in] lod The level-of-detail parameter, \p lod, selects
    //! the approximation level. See DataMgr.
    //
    VAPoR::Grid *GetVariable(size_t ts, string varname, int level, int lod, bool lock = false);

    //! Read and return a variable hyperslab
    //!
    //! This method is identical to the GetVariable() method, however,
    //! a subregion is specified in the user coordinate system.
    //! \p min and \p max specify the minimum and maximum extents of an
    //! axis-aligned bounding box containing the region of interest. The
    //! VAPoR::Grid object returned contains the intersection
    //! between the
    //! specified
    //! hyperslab and the variable's spatial domain (which is not necessarily
    //! rectangular or axis-aligned).
    //! If the requested hyperslab lies entirely outside of the domain of the
    //! requested variable NULL is returned
    //!
    //! \param[in] min A one, two, or three element array specifying the
    //! minimum extents, in user coordinates, of an axis-aligned box defining
    //! the region-of-interest. The spatial dimensionality of the variable
    //! determines the number of elements in \p min.
    //!
    //! \param[in] max A one, two, or three element array specifying the
    //! maximum extents, in user coordinates, of an axis-aligned box defining
    //! the region-of-interest. The spatial dimensionality of the variable
    //! determines the number of elements in \p max.
    //!
    //! \note The Grid structure returned is allocated from the heap.
    //! it is the caller's responsiblity to delete the returned object
    //! when it is no longer in use.
    //!
    VAPoR::Grid *GetVariable(size_t ts, string varname, int level, int lod, std::vector<double> min, std::vector<double> max, bool lock = false);

    VAPoR::Grid *GetVariable(size_t ts, string varname, int level, int lod, std::vector<size_t> min, std::vector<size_t> max, bool lock = false);

    //! Compute the coordinate extents of a variable
    //!
    //! This method finds the spatial domain extents of a variable
    //!
    //! This method returns the min and max extents
    //! specified in user
    //! coordinates, of the smallest axis-aligned bounding box that is
    //! guaranteed to contain
    //! the variable indicated by \p varname, and the given refinement level,
    //! \p level
    //!
    int GetVariableExtents(size_t ts, string varname, int level, std::vector<double> &min, std::vector<double> &max);

    int GetDataRange(size_t ts, string varname, int level, int lod, std::vector<double> &range);

    //! \copydoc DC::GetDimLensAtLevel()
    //!
    //! \param[in] spatial A boolean, if true, indicates that only the variable's
    //! spatial dimensions and block size should be returned. A time varying
    //! dimension, if one exists, is not included
    //!
    virtual int GetDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

    //! Unlock a floating-point region of memory
    //!
    //! Decrement the lock counter associatd with a
    //! region of memory, and if zero,
    //! unlock region of memory previously locked with GetVariable().
    //! When the lock counter reaches zero the region is simply
    //! marked available for
    //! internal garbage collection during subsequent GetVariable() calls
    //!
    //! \param[in] rg A pointer to a Grid previosly
    //! returned by GetVariable()
    //!
    //! \retval status Returns a non-negative value on success
    //!
    //! \sa GetVariable()
    //
    void UnlockGrid(const VAPoR::Grid *rg);

    //! \copydoc DC::GetNumDimensions(
    //!   string varname, size_t &ndim
    //! ) const;
    //
    bool GetNumDimensions(string varname, size_t &ndim) const;

    //! \copydoc DC:GetVarTopologyDim()
    //!
    size_t GetVarTopologyDim(string varname) const;

    //! Clear the memory cache
    //!
    //! This method clears the internal memory cache of all entries
    //
    void Clear();

    //! Returns true if indicated data volume is available
    //!
    //! Returns true if the variable identified by the timestep, variable
    //! name, refinement level, and level-of-detail is present in
    //! the data set. Returns false if
    //! the variable is not available.
    //!
    //! \param[in] ts A valid time step between 0 and GetNumTimesteps()-1
    //! \param[in] varname A valid variable name
    //! \param[in] level Refinement level requested.
    //! \param[in] lod Compression level of detail requested.
    //! refinement level contained in the DC.
    //
    virtual bool VariableExists(size_t ts, string varname, int level = 0, int lod = 0) const;

    //! \copydoc DC::GetMapProjection(
    //!   string varname
    //! ) const;
    //
    virtual string GetMapProjection(string varname) const
    {
        assert(_dc != NULL);
        return (_dc->GetMapProjection(varname));
    }

    //! \copydoc DC::GetMapProjection() const;
    //
    virtual string GetMapProjection() const
    {
        assert(_dc != NULL);
        return (_dc->GetMapProjection());
    }

    //! Return default Map Projection string, if any
    //!
    //! This method returns the default Proj4 Map Projection string,
    //! if any are define. The VDC can support multiple Proj4 projection
    //! strings, one per variable. This method returns the first
    //! Proj4 projection string found.
    //
    string GetDefaultMapProjection() const { return (_defaultMapProjectionStr); }

    #ifdef DEAD

    //!
    //! Add a pipeline stage to produce derived variables
    //!
    //! Add a new pipline stage for derived variable calculation. If a
    //! pipeline already exists with the same
    //! name it is replaced. The output variable names are added to
    //! the list of variables available for this data
    //! set (see GetVariables3D, etc.).
    //!
    //! An error occurs if:
    //!
    //! \li The output variable names match any of the native variable
    //! names - variable names returned via _GetVariables3D(), etc.
    //! \li The output variable names match the output variable names
    //! of pipeline stage previously added with NewPipeline()
    //! \li A circular dependency is introduced by adding \p pipeline
    //!
    //! \retval status A negative int is returned on failure.
    //!
    int NewPipeline(PipeLine *pipeline);

    //!
    //! Remove the named pipline if it exists. Otherwise this method is a
    //! no-op
    //!
    //! \param[in] name The name of the pipeline as returned by
    //! PipeLine::GetName()
    //!
    void RemovePipeline(string name);
    #endif

    //! Return true if the named variable is the output of a pipeline
    //!
    //! This method returns true if \p varname matches a variable name
    //! in the output list (PipeLine::GetOutputs()) of any pipeline added
    //! with NewPipeline()
    //!
    //! \sa NewPipeline()
    //
    bool IsVariableDerived(string varname) const;

    //! Return true if the named variable is availble from the derived
    //! classes data access methods.
    //!
    //! A return value of true does not imply that the variable can
    //! be read (\see VariableExists()), only that it is part of the
    //! data set known to the derived class
    //!
    //! \sa NewPipeline()
    //
    bool IsVariableNative(string varname) const;

    //! Purge the cache of a variable
    //!
    //! \param[in] varname is the variable name
    //!
    void PurgeVariable(string varname);

    class BlkExts {
    public:
        BlkExts();
        BlkExts(const std::vector<size_t> &bmin, const std::vector<size_t> &bmax);

        void Insert(const std::vector<size_t> &bcoord, const std::vector<double> &min, const std::vector<double> &max);

        bool Intersect(const std::vector<double> &min, const std::vector<double> &max, std::vector<size_t> &bmin, std::vector<size_t> &bmax) const;

        friend std::ostream &operator<<(std::ostream &o, const BlkExts &b);

    private:
        std::vector<size_t>              _bmin;
        std::vector<size_t>              _bmax;
        std::vector<std::vector<double>> _mins;
        std::vector<std::vector<double>> _maxs;
    };

private:
    //
    // Cache for various metadata attributes
    //
    class VarInfoCache {
    public:
        //
        //
        void Set(size_t ts, std::vector<string> varnames, int level, int lod, string key, const std::vector<size_t> &values);
        void Set(size_t ts, string varname, int level, int lod, string key, const std::vector<size_t> &values)
        {
            std::vector<string> varnames;
            varnames.push_back(varname);
            Set(ts, varnames, level, lod, key, values);
        }

        bool Get(size_t ts, std::vector<string> varnames, int level, int lod, string key, std::vector<size_t> &values) const;

        bool Get(size_t ts, string varname, int level, int lod, string key, std::vector<size_t> &values) const
        {
            std::vector<string> varnames;
            varnames.push_back(varname);
            return Get(ts, varnames, level, lod, key, values);
        }

        void PurgeSize_t(size_t ts, std::vector<string> varnames, int level, int lod, string key);
        void PurgeSize_t(size_t ts, string varname, int level, int lod, string key)
        {
            std::vector<string> varnames;
            varnames.push_back(varname);
            PurgeSize_t(ts, varnames, level, lod, key);
        }

        void Set(size_t ts, std::vector<string> varnames, int level, int lod, string key, const std::vector<double> &values);
        void Set(size_t ts, string varname, int level, int lod, string key, const std::vector<double> &values)
        {
            std::vector<string> varnames;
            varnames.push_back(varname);
            Set(ts, varnames, level, lod, key, values);
        }

        bool Get(size_t ts, std::vector<string> varnames, int level, int lod, string key, std::vector<double> &values) const;

        bool Get(size_t ts, string varname, int level, int lod, string key, std::vector<double> &values) const
        {
            std::vector<string> varnames;
            varnames.push_back(varname);
            return Get(ts, varnames, level, lod, key, values);
        }

        void PurgeDouble(size_t ts, std::vector<string> varnames, int level, int lod, string key);
        void PurgeDouble(size_t ts, string varname, int level, int lod, string key)
        {
            std::vector<string> varnames;
            varnames.push_back(varname);
            PurgeDouble(ts, varnames, level, lod, key);
        }

        void Set(size_t ts, std::vector<string> varnames, int level, int lod, string key, const std::vector<void *> &values);
        void Set(size_t ts, string varname, int level, int lod, string key, const std::vector<void *> &values)
        {
            std::vector<string> varnames;
            varnames.push_back(varname);
            Set(ts, varnames, level, lod, key, values);
        }

        bool Get(size_t ts, std::vector<string> varnames, int level, int lod, string key, std::vector<void *> &values) const;

        bool Get(size_t ts, string varname, int level, int lod, string key, std::vector<void *> &values) const
        {
            std::vector<string> varnames;
            varnames.push_back(varname);
            return Get(ts, varnames, level, lod, key, values);
        }
        bool Get(string hash, std::vector<void *> &values) const
        {
            values.clear();
            std::map<string, std::vector<void *>>::const_iterator itr;
            itr = _cacheVoidPtr.find(hash);
            if (itr != _cacheVoidPtr.end()) {
                values = itr->second;
                return (true);
            }
            return (false);
        }

        void PurgeVoidPtr(size_t ts, std::vector<string> varnames, int level, int lod, string key);
        void PurgeVoidPtr(size_t ts, string varname, int level, int lod, string key)
        {
            std::vector<string> varnames;
            varnames.push_back(varname);
            PurgeVoidPtr(ts, varnames, level, lod, key);
        }

        vector<string> GetVoidPtrHash() const
        {
            vector<string>                                        keys;
            std::map<string, std::vector<void *>>::const_iterator itr;
            for (itr = _cacheVoidPtr.begin(); itr != _cacheVoidPtr.end(); ++itr) { keys.push_back(itr->first); }
            return (keys);
        }

        void Clear()
        {
            _cacheSize_t.clear();
            _cacheDouble.clear();
            _cacheVoidPtr.clear();
        }

        static string _make_hash(string key, size_t ts, std::vector<string> cvars, int level, int lod);

    private:
        std::map<string, std::vector<size_t>> _cacheSize_t;
        std::map<string, std::vector<double>> _cacheDouble;
        std::map<string, std::vector<void *>> _cacheVoidPtr;
    };

    string _format;
    int    _nthreads;
    size_t _mem_size;

    DC *_dc;

    std::vector<double> _timeCoordinates;
    string              _defaultMapProjectionStr;

    typedef struct {
        size_t              ts;
        string              varname;
        int                 level;
        int                 lod;
        std::vector<size_t> bmin;
        std::vector<size_t> bmax;
        int                 lock_counter;
        void *              blks;
    } region_t;

    // a list of all allocated regions
    std::list<region_t> _regionsList;

    VAPoR::BlkMemMgr *_blk_mem_mgr;

    std::vector<PipeLine *> _PipeLines;

    mutable VarInfoCache      _varInfoCache;
    std::map<string, BlkExts> _blkExtsCache;

    int _get_coord_vars(string varname, std::vector<string> &scvars, string &tcvar) const;

    int _get_time_coordinates(std::vector<double> &timecoords);

    int _get_default_projection(string &projection);

    VAPoR::RegularGrid *_make_grid_regular(const std::vector<size_t> &dims, const std::vector<float *> &blkvec, const std::vector<size_t> &bs, const std::vector<size_t> &bmin,
                                           const std::vector<size_t> &bmax) const;

    VAPoR::StretchedGrid *_make_grid_stretched(const std::vector<size_t> &dims, const std::vector<float *> &blkvec, const std::vector<size_t> &bs, const std::vector<size_t> &bmin,
                                               const std::vector<size_t> &bmax) const;

    VAPoR::LayeredGrid *_make_grid_layered(const std::vector<size_t> &dims, const std::vector<float *> &blkvec, const std::vector<size_t> &bs, const std::vector<size_t> &bmin,
                                           const std::vector<size_t> &bmax) const;

    VAPoR::CurvilinearGrid *_make_grid_curvilinear(int level, int lod, const vector<DC::CoordVar> &cvarsinfo, const std::vector<size_t> &dims, const std::vector<float *> &blkvec,
                                                   const std::vector<size_t> &bs, const std::vector<size_t> &bmin, const std::vector<size_t> &bmax);

    void _ugrid_setup(const DC::DataVar &var, std::vector<size_t> &vertexDims, std::vector<size_t> &faceDims, std::vector<size_t> &edgeDims,
                      UnstructuredGrid::Location &location,    // node,face, edge
                      size_t &maxVertexPerFace, size_t &maxFacePerVertex, long &vertexOffset, long &faceOffset) const;

    UnstructuredGrid2D *_make_grid_unstructured2d(int level, int lod, const DC::DataVar &dvarinfo, const vector<DC::CoordVar> &cvarsinfo, const vector<size_t> &dims, const vector<float *> &blkvec,
                                                  const vector<size_t> &bs, const vector<size_t> &bmin, const vector<size_t> &bmax, const vector<int *> &conn_blkvec, const vector<size_t> &conn_bs,
                                                  const vector<size_t> &conn_bmin, const vector<size_t> &conn_bmax);

    VAPoR::Grid *_make_grid(int level, int lod, const VAPoR::DC::DataVar &var, const std::vector<size_t> &roi_dims, const std::vector<size_t> &dims, const std::vector<float *> &blkvec,
                            const std::vector<std::vector<size_t>> &bsvec, const std::vector<std::vector<size_t>> &bminvec, const std::vector<std::vector<size_t>> &bmaxvec,
                            const vector<int *> &conn_blkvec, const vector<vector<size_t>> &conn_bsvec, const vector<vector<size_t>> &conn_bminvec, const vector<vector<size_t>> &conn_bmaxvec);

    enum GridType { UNDEFINED = 0, REGULAR, STRETCHED, LAYERED, CURVILINEAR, UNSTRUC_2D, UNSTRUC_LAYERED };
    GridType _get_grid_type(const DC::DataVar &var, const vector<DC::CoordVar> &cvarsinfo) const;

    GridType _get_grid_type(string varname) const;

    int _find_bounding_grid(size_t ts, string varname, int level, int lod, std::vector<double> min, std::vector<double> max, std::vector<size_t> &min_ui, std::vector<size_t> &max_ui);

    int _setupCoordVecs(size_t ts, string varname, int level, int lod, const vector<size_t> &min, const vector<size_t> &max, vector<string> &varnames, vector<size_t> &roi_dims,
                        vector<vector<size_t>> &dims_at_levelvec, vector<vector<size_t>> &bsvec, vector<vector<size_t>> &bs_at_levelvec, vector<vector<size_t>> &bminvec,
                        vector<vector<size_t>> &bmaxvec) const;

    int _setupConnVecs(size_t ts, string varname, int level, int lod, const vector<size_t> &min, const vector<size_t> &max, vector<string> &varnames, vector<vector<size_t>> &dims_at_levelvec,
                       vector<vector<size_t>> &bsvec, vector<vector<size_t>> &bs_at_levelvec, vector<vector<size_t>> &bminvec, vector<vector<size_t>> &bmaxvec) const;

    VAPoR::Grid *_getVariable(size_t ts, string varname, int level, int lod, bool lock, bool dataless);

    VAPoR::Grid *_getVariable(size_t ts, string varname, int level, int lod, std::vector<size_t> min, std::vector<size_t> max, bool lock, bool dataless);

    template<typename T> T *_get_region_from_cache(size_t ts, string varname, int level, int lod, const std::vector<size_t> &bmin, const std::vector<size_t> &bmax, bool lock);

    template<typename T>
    T *_get_region_from_fs(size_t ts, string varname, int level, int lod, const std::vector<size_t> &bs, const std::vector<size_t> &bmin, const std::vector<size_t> &bmax, bool lock);

    template<typename T>
    T *_get_region(size_t ts, string varname, int level, int nlevels, int lod, int nlods, const std::vector<size_t> &bs, const std::vector<size_t> &bmin, const std::vector<size_t> &bmax, bool lock);

    template<typename T>
    int _get_regions(size_t ts, const std::vector<string> &varnames, int level, int lod, bool lock, const std::vector<std::vector<size_t>> &bsvec, const std::vector<std::vector<size_t>> &bminvec,
                     const std::vector<std::vector<size_t>> &bmaxvec, std::vector<T *> &blkvec);

    void _unlock_blocks(const void *blks);

    std::vector<string> _get_native_variables() const;
    std::vector<string> _get_derived_variables() const;

    void *_alloc_region(size_t ts, string varname, int level, int lod, std::vector<size_t> bmin, std::vector<size_t> bmax, std::vector<size_t> bs, int element_sz, bool lock, bool fill);

    void _free_region(size_t ts, string varname, int level, int lod, std::vector<size_t> bmin, std::vector<size_t> bmax);

    bool _free_lru();
    void _free_var(string varname);

    int _level_correction(string varname, int &level) const;
    int _lod_correction(string varname, int &lod) const;

    const KDTreeRG *_getKDTree2D(int level, int lod, const vector<DC::CoordVar> &cvarsinfo, const Grid &xg, const Grid &yg);
};

};    // namespace VAPoR
#endif
