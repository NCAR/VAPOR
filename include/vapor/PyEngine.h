#include <vector>
#include <Python.h>
#include <vapor/DataMgr.h>
#include <vapor/DC.h>

#pragma once

namespace VAPoR {

//! \class PyEngine
//! \brief A class for managing derived variables computed with Python
//! \author John Clyne
//!
//! This class provides a means to manage derived variables on the DataMgr
//! calculated using Python. It allows the user of this class to define
//! derived variables that execute Python scripts and installs the derived
//! variable on the DataMgr class. The Python script may operate on
//! input variables managed by the DataMgr
//
class VDF_API PyEngine : public Wasp::MyBase {
public:
    //! Constructor for PyEngine class
    //!
    //! \param[in] dataMgr A pointer to a DataMgr instance upon which derived
    //! variables created by this class will be managed.
    //
    PyEngine(DataMgr *dataMgr)
    {
        assert(dataMgr != NULL);
        _dataMgr = dataMgr;
    }

    ~PyEngine();

    //! Initialize the class
    //!
    //! This static initializer must be called to initialize the Python C API
    //! at least once prior to using
    //! other PyEngine class methods.
    //!
    //! \retval status A negative int is returned on failure and an error
    //! message will be logged with MyBase::SetErrMsg()
    //
    static int Initialize();

    //! Add new derived variable(s) to the DataMgr
    //!
    //! This method adds one or more derived variables to the DataMgr specified
    //! by the constructor. Each derived variable is calculated by executing
    //! the same Python script specified by \p script. I.e. a single Python
    //! script may compute multiple variables.
    //!
    //! \param[in] name A string identifier for the collection of derived variables
    //! computed by \p script
    //!
    //! \param[in] script A Python (NumPy) script that will be invoked each time
    //! one of the variables listed in \p outputs is accessed. The scope of
    //! the script will contain NumPy Array (numpy.array) variables named
    //! in \p inputs.
    //!
    //! \param[in] inputs A list of input DataMgr variable names. The named
    //! DataMgr variables will be made available in the scope of the Python
    //! script as NumPy Arrays.
    //!
    //! \param[in] outputs A list of derived DataMgr variable names. The named
    //! variables are expected to be computed by \p script as NumPy Arrays
    //! and will appear as DataMgr derived variables.
    //!
    //! \param[in] outMeshes A list of output mesh names, one for each output
    //! variable listed in \p outputs. The output mesh names must be known
    //! to the DataMgr. Each output variable created by \p script is expected
    //! to be sampled by the named mesh. See DataMgr::GetMesh()
    //!
    //! \retval status A negative integer is returned on failure and an error
    //! message is reported with MyBase::SetErrMsg(). AddFunction() will fail
    //! if any of the output variables named in \p outputs already exist
    //! in the DataMgr as returned by DataMgr::GetDataVarNames(), or if any of
    //! the output mesh names in \p outMeshes are not known to the
    //! DataMgr (see DataMgr::GetMeshNames())
    //!
    //! \note The Python script \p script is executed when one of the output
    //! variables is read. Depending on the region requested only a subset
    //! of the DataMgr variable may be provided to \p script as a NumPy
    //! array. Currently this occurs if all of the input variables named
    //! by \p inputs and the requested output variable are sampled on the
    //! same mesh.
    //
    int AddFunction(string name, string script, const vector<string> &inputs, const vector<string> &outputs, const vector<string> &outMeshes);

    //! Remove a previously defined function
    //!
    //! This method removes the function previously created by AddFunction()
    //! and named by \p name. All of the associated derived variables are
    //! removed from the DataMgr. The method is a no-op if \p name is not
    //! an active function.
    //
    void RemoveFunction(string name);

    //! Return a list of all active function names
    //!
    //! This method returns a vector of names for all active (not previously
    //! removed) functions created with AddFunction()
    //!
    //! \sa AddFunction();
    //!
    std::vector<string> GetFunctionNames() const;

    //! Return the script for a named function
    //!
    //! This method returns as a string the NumPy script associated with the
    //! function named by \p name.
    //!
    //! \retval script Returns the Python script bound to \p name. Any empty
    //! string is returned if \p name is not defined.
    //!
    //! \sa AddFunction(), RemoveFunction()
    //
    string GetFunctionScript(string name) const;

    //! Execute a NumPy script
    //!
    //! This static method executes the NumPy script \p script and copies the
    //! outputs of the Python NumPy Arrays named by \p outputVarNames
    //! into the regions of memory provided by \p outputVarArrays. The contents
    //! of the memory referenced by \p inputVarArrays are made available to
    //! \p script as inputs in the form of NumPy Arrays.
    //!
    //! \param[in] script A Python (NumPy) script that is expected to operate on
    //! the NumPy Arrays listed in \p inputVarNames and store outputs in the
    //! NumPy arrays listed in \p outputVarNames
    //!
    //! \param[in] inputVarNames A list of NumPy Array names that will be made
    //! available in the scope of the executed Python script, \p script.
    //!
    //! \param[in] inputVarDims A list of dimensions for each of the possibly
    //! multi-dimensional NumPy arrays named by \p inputVarNames
    //!
    //! \param[in] inputVarArrays A list of regions of memory for each
    //! input NumPy Array that will be copied in the Python environment prior
    //! to executing \p script. The size of the region copy is given by the
    //! dimensions in \p inputVarDims
    //!
    //! \param[in] outputVarNames A list of NumPy Array names that are
    //! expected to be created within the scope of the Python script  \p script
    //! The required dimesions of each array is given by \p outputVarDims
    //!
    //! \param[in] outputVarDims A list of dimensions for each of the possibly
    //! multi-dimensional NumPy arrays named by \p outputVarNames.
    //!
    //! \param[in] outputVarArrays A list of regions of memory for each
    //! output NumPy Array that will be copied out of the Python environment after
    //! executing \p script. The size of the region copied is given by the
    //! dimensions in \p outputVarDims
    //!
    static int Calculate(const string &script, vector<string> inputVarNames, vector<vector<size_t>> inputVarDims, vector<float *> inputVarArrays, vector<string> outputVarNames,
                         vector<vector<size_t>> outputVarDims, vector<float *> outputVarArrays);

private:
    class VDF_API DerivedPythonVar : public DerivedDataVar {
    public:
        DerivedPythonVar(string varName, string units, DC::XType type, string mesh, string time_coord_var, bool hasMissing, std::vector<string> inNames, string script, DataMgr *dataMgr);

        ~DerivedPythonVar() {}

        int Initialize();

        bool GetBaseVarInfo(DC::BaseVar &var) const;

        std::vector<string> GetInputs() const { return (_inNames); }

        int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

        int OpenVariableRead(size_t ts, int level = 0, int lod = 0);

        int CloseVariable(int fd);

        int ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) { return (ReadRegion(fd, min, max, region)); }

        int ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

        bool VariableExists(size_t ts, int reflevel, int lod) const;

        bool GetDataVarInfo(DC::DataVar &cvar) const;

    private:
        DC::DataVar         _varInfo;
        std::vector<string> _inNames;
        string              _script;
        DataMgr *           _dataMgr;
        DC::FileTable       _fileTable;
        vector<size_t>      _dims;
        bool                _readSubsetFlag;

        int _readRegionAll(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

        int _readRegionSubset(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);
    };

    class func_c {
    public:
        func_c() {}
        func_c(const string &name, const string &script, const std::vector<string> &inputVarNames, const std::vector<string> &outputVarNames, const std::vector<string> &outputMeshNames,
               const std::vector<DerivedPythonVar *> &derivedVars)
        : _name(name), _script(script), _inputVarNames(inputVarNames), _outputVarNames(outputVarNames), _outputMeshNames(outputMeshNames), _derivedVars(derivedVars)
        {
        }

        string                          _name;
        string                          _script;
        std::vector<string>             _inputVarNames;
        std::vector<string>             _outputVarNames;
        std::vector<string>             _outputMeshNames;
        std::vector<DerivedPythonVar *> _derivedVars;
    };

    std::map<string, func_c> _functions;
    DataMgr *                _dataMgr;
    static bool              _isInitialized;

    PyEngine() : _dataMgr(NULL) {}

    static void _cleanupDict(PyObject *mainDict, PyObject *copyDict);

    static int _c2python(PyObject *dict, vector<string> inputVarNames, vector<vector<size_t>> inputVarDims, vector<float *> inputVarArrays);

    static int _python2c(PyObject *dict, vector<string> outputVarNames, vector<vector<size_t>> outputVarDims, vector<float *> outputVarArrays);

    bool _validOutputVar(string name) const;
    int  _checkOutVars(const vector<string> &outputVarNames) const;
    bool _validOutputMesh(string name) const;
    int  _checkOutMeshes(const vector<string> &outputMeshNames) const;

    string _getTimeCoordVarName(const vector<string> &varNames) const;
};
};    // namespace VAPoR
