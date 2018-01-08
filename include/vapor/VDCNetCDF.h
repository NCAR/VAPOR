#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include "vapor/VDC.h"
#include "vapor/WASP.h"

#ifndef _VDCNetCDF_H_
#define _VDCNetCDF_H_

namespace VAPoR {

//! \class VDCNetCDF
//!	\ingroup Public_VDC
//!
//! \brief Implements the VDC
//! abstract class, providing storage of VDC data
//! in NetCDF files.
//!
//! \author John Clyne
//! \date    July, 2014
//!
//! Implements the VDC abstract class, providing storage of VDC data
//! in NetCDF files. Data (variables) are stored in multiple NetCDF files.
//! The distribution of variables to files is described by GetPath().
//!
class VDF_API VDCNetCDF : public VAPoR::VDC {
  public:
    //! Class constructor
    //!
    //! \param[in] numthreads Number of parallel execution threads
    //! to be run during encoding and decoding of compressed data. A value
    //! of 0, the default, indicates that the thread count should be
    //! determined by the environment in a platform-specific manner, for
    //! example using sysconf(_SC_NPROCESSORS_ONLN) under *nix OSes.
    //!
    //! \param[in] master_theshold Variables that are either compressed, or whose
    //! total number of elements are larger than \p master_theshold, will
    //! not be stored in the master file. Ignored if the file is open
    //! for appending or reading.
    //!
    //! \param[in] variable_threshold Variables not stored in the master
    //! file and whose
    //! total number of elements are larger than \p variable_threshold
    //! will be stored with one time step per file. Ignored if the file is open
    //! for appending or reading.
    //
    VDCNetCDF(
        int numthreads = 0,
        size_t master_theshold = 10 * 1024 * 1024,
        size_t variable_threshold = 100 * 1024 * 1024);
    virtual ~VDCNetCDF();

    //! \copydoc DC:GetHyperSliceInfo()
    //!
    //! Override base class to ensure hyperslices are block aligned
    //!
    virtual int GetHyperSliceInfo(
        string varname, int level, std::vector<size_t> &dims,
        size_t &nslice);

    //! Return path to the data directory
    //!
    //! Return the file path to the data directory associated with the
    //! master file named by \p path. Data files, those NetCDF files
    //! containing coordinate and data variables, are stored in separate
    //! files from
    //! the VDC master file (See VDC::Initialize()). The data files reside
    //! under the directory returned by this command.
    //!
    //! \param[in] path Path to VDC master file
    //!
    //! \retval dir : Path to the data directory
    //! \sa Initialize(), GetPath(), DataDirExists()
    //
    static string GetDataDir(string path);

    //! \copydoc VDC::GetPath()
    //!
    //! \par Algorithm
    //! If the size of a variable (total number of elements) is less than
    //! GetMasterThreshold() and the variable is not compressed it will
    //! be stored in the file master file. Otherwise, the variable will
    //! be stored in either the coordinate variable or data variable
    //! directory, as appropriate. Variables stored in coordinate or
    //! data directories are stored one variable per file. If the size
    //! of the variable is less than GetVariableThreshold() and the
    //! variable is time varying multiple time steps may be saved in a single
    //! file. If the variable is compressed each compression level is stored
    //! in a separate file.
    //!
    //! \sa VDCNetCDF::VDCNetCDF(), GetMasterThreshold(), GetVariableThreshold()
    //!
    virtual int GetPath(
        string varname, size_t ts, string &path, size_t &file_ts,
        size_t &max_ts) const;

    //! \copydoc VDC::GetDimLensAtLevel()
    //
    virtual int getDimLensAtLevel(
        string varname, int level, std::vector<size_t> &dims_at_level,
        vector<size_t> &bs_at_level) const;

    //! Return true if a data directory exists for the master file
    //! named by \p path
    //!
    //! \param[in] path Path to VDC master file
    //!
    //! \sa Initialize(), GetPath(), GetDataDir();
    //
    static bool DataDirExists(string path);

    //! Initialize the VDCNetCDF class
    //! \copydoc VDC::Initialize()
    //!
    //! \param[in] chunksizehint : NetCDF chunk size hint.  A value of
    //! zero results in NC_SIZEHINT_DEFAULT being used.
    //
    virtual int Initialize(
        const vector<string> &paths, const vector<string> &options,
        AccessMode mode, size_t chunksizehint = 0);
    virtual int Initialize(
        string path, const vector<string> &options,
        AccessMode mode, size_t chunksizehint = 0) {
        std::vector<string> paths;
        paths.push_back(path);
        return (Initialize(paths, options, mode, chunksizehint));
    }

    //! Return the master file size threshold
    //!
    //! \sa VDCNetCDF::VDCNetCDF(), GetVariableThreshold()
    //
    size_t GetMasterThreshold() const { return _master_threshold; };

    //! Return the variable size  threshold
    //!
    //! \sa VDCNetCDF::VDCNetCDF(), GetMasterThreshold()
    //
    size_t GetVariableThreshold() const { return _variable_threshold; };

    //! \copydoc VDC::OpenVariableWrite()
    //
    int OpenVariableWrite(size_t ts, string varname, int lod = -1);

    int CloseVariableWrite() {
        return (closeVariable());
    };

    //! \copydoc VDC::Write()
    //
    int Write(const float *region) {
        return (_writeTemplate(region));
    }
    int Write(const int *region) {
        return (_writeTemplate(region));
    }

    int WriteSlice(const float *slice) {
        return (_writeSliceTemplate(_ofi._wasp, slice));
    };
    int WriteSlice(const int *slice) {
        return (_writeSliceTemplate(_ofi._wasp, slice));
    };
    int WriteSlice(const unsigned char *slice) {
        return (_writeSliceTemplate(_ofi._wasp, slice));
    }

    //! \copydoc VDC::PutVar()
    //
    int PutVar(string varname, int lod, const float *data) {
        return (_putVarTemplate(varname, lod, data));
    }
    int PutVar(string varname, int lod, const int *data) {
        return (_putVarTemplate(varname, lod, data));
    }

    //! \copydoc VDC::PutVar()
    //
    int PutVar(size_t ts, string varname, int lod, const float *data) {
        return (_putVarTemplate(ts, varname, lod, data));
    }
    int PutVar(size_t ts, string varname, int lod, const int *data) {
        return (_putVarTemplate(ts, varname, lod, data));
    }

    int CopyVar(DC &dc, string varname, int srclod, int dstlod);
    int CopyVar(DC &dc, size_t ts, string varname, int srclod, int dstlod);

    //! \copydoc VDC::CompressionInfo()
    //
    bool CompressionInfo(
        std::vector<size_t> bs, string wname, size_t &nlevels, size_t &maxcratio) const;

    //! Enable or disable the NetCDF fill-value mode
    //!
    //! Enable or disable the NetCDF fill-value mode
    //!
    //! \param[in] fillmode NC_FILL as 0x0 or NC_NOFILL as 0x100
    //!
    //! \retval err: NC_NOERR on success
    //
    int SetFill(int fillmode);

  protected:
#ifndef DOXYGEN_SKIP_THIS
    virtual int _WriteMasterMeta();
    virtual int _ReadMasterMeta();
#endif

    int openVariableRead(
        size_t ts, string varname, int level = 0, int lod = -1);

    int closeVariable();

    int read(float *data);

    int read(int *data);

    int readSlice(float *slice);
    int readSlice(unsigned char *slice);

    int readRegion(
        const std::vector<size_t> &min,
        const std::vector<size_t> &max, float *region);
    int readRegion(
        const std::vector<size_t> &min,
        const std::vector<size_t> &max, int *region);

    int readRegionBlock(
        const vector<size_t> &min, const vector<size_t> &max, float *region);
    int readRegionBlock(
        const vector<size_t> &min, const vector<size_t> &max, int *region);

    virtual bool variableExists(
        size_t ts,
        string varname,
        int reflevel = 0,
        int lod = 0) const;

  private:
    int _version;
    WASP *_master; // Master NetCDF file

    class open_file_info {
      public:
        open_file_info() {
            _wasp = NULL;
            _write = false;
            _dims.clear();
            _bs.clear();
            _slice_num = 0;
            _ts = 0;
            _file_ts = 0;
            _varname.clear();
            _level = 0;
        };
        open_file_info(
            WASP *wasp, bool write, vector<size_t> dims,
            vector<size_t> bs, size_t slice_num,
            size_t ts, size_t file_ts, string varname, int level) : _wasp(wasp),
                                                                    _write(write),
                                                                    _dims(dims),
                                                                    _bs(bs),
                                                                    _slice_num(slice_num),
                                                                    _ts(ts),
                                                                    _file_ts(file_ts),
                                                                    _varname(varname),
                                                                    _level(level){};

        WASP *_wasp; // Currently opened data file
        bool _write; // opened for writing?
        vector<size_t> _dims;
        vector<size_t> _bs;
        size_t _slice_num; // index of current slice for WriteSlice, ReadSlice
        size_t _ts;        // global time step of current open variable
        size_t _file_ts;   // local (within file) time step of current open var
        string _varname;   // name of current open variable
        int _level;
    };
    open_file_info _ofi;     // currently open data or coordinate variable
    open_file_info _ofimask; // current opened mask variable

    Wasp::SmartBuf _sb_slice_buffer;
    Wasp::SmartBuf _mask_buffer;

    size_t _chunksizehint; // NetCDF chunk size hint for file creates
    size_t _master_threshold;
    size_t _variable_threshold;
    int _nthreads;

    int _WriteMasterDimensions();
    int _WriteMasterAttributes(
        string prefix, const map<string, Attribute> &atts);
    int _WriteMasterAttributes();
    int _WriteMasterMeshDefs();
    int _WriteMasterBaseVarDefs(string prefix, const BaseVar &var);
    int _WriteMasterCoordVarsDefs();
    int _WriteMasterDataVarsDefs();

    template <class T>
    int _writeTemplate(const T *data);

    template <class T>
    int _writeSliceTemplate(WASP *file, const T *slice);

    int _ReadMasterDimensions();
    int _ReadMasterAttributes(
        string prefix, map<string, Attribute> &atts);
    int _ReadMasterAttributes();
    int _ReadMasterMeshDefs();
    int _ReadMasterBaseVarDefs(string prefix, BaseVar &var);
    int _ReadMasterCoordVarsDefs();
    int _ReadMasterDataVarsDefs();

    template <class T>
    int _ReadSlice(WASP *file, T *slice);

    int _PutAtt(
        WASP *ncdf,
        string varname,
        string tag,
        const Attribute &attr);

    int _WriteAttributes(
        WASP *wasp, string varname, const map<string, Attribute> &atts);
    int _DefBaseVar(WASP *ncdf, const VDC::BaseVar &var, size_t max_ts);
    int _DefDataVar(WASP *ncdf, const VDC::DataVar &var, size_t max_ts);
    int _DefCoordVar(WASP *ncdf, const VDC::CoordVar &var, size_t max_ts);

    bool _var_in_master(const VDC::BaseVar &var) const;

    string _get_mask_varname(string varname) const;

    unsigned char *_read_mask_var(
        vector<size_t> start, vector<size_t> count);

    WASP *_OpenVariableRead(
        size_t ts, string varname, int clevel, int lod,
        size_t &file_ts);

    int _ReadHelper(
        vector<size_t> &start,
        vector<size_t> &count) const;

    template <class T>
    int _putVarTemplate(string varname, int lod, const T *data);

    template <class T>
    int _putVarTemplate(
        size_t ts, string varname, int lod, const T *data);

    int _copyVar0d(DC &dc, size_t ts, const BaseVar &varInfo);

    template <class T>
    int _copyVarHelper(
        DC &dc, vector<size_t> &buffer_dims,
        vector<size_t> &src_hslice_dims, vector<size_t> &dst_hslice_dims,
        size_t src_nslice, size_t dst_nslice, T *buffer);
};
}; // namespace VAPoR

#endif
