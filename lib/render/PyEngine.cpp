#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <new>
#include <vapor/utils.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/MyPython.h>
#include <vapor/PyEngine.h>
using namespace Wasp;
using namespace VAPoR;

#define NPY_NO_DEPRECATED_API NPY_API_VERSION
#include <numpy/arrayobject.h>

namespace {

void free_arrays(vector<float *> &arrays)
{
    for (int i = 0; i < arrays.size(); i++) {
        if (arrays[i]) delete[] arrays[i];
    }
}

void free_arrays(vector<float *> &inputVarArrays, vector<float *> &outputVarArrays)
{
    free_arrays(inputVarArrays);
    free_arrays(outputVarArrays);
}

int alloc_arrays(const vector<vector<size_t>> &dimsVectors, vector<float *> &arrays)
{
    size_t nElements = 1;
    for (int i = 0; i < dimsVectors.size(); i++) { nElements *= VProduct(dimsVectors[i]); }

    float *buf = new (nothrow) float[nElements];
    if (!buf) return (-1);

    float *bufptr = buf;
    for (int i = 0; i < dimsVectors.size(); i++) {
        arrays.push_back(bufptr);
        bufptr += VProduct(dimsVectors[i]);
    }

    return (0);
}

int alloc_arrays(const vector<vector<size_t>> &inputVarDims, const vector<vector<size_t>> &outputVarDims, vector<float *> &inputVarArrays, vector<float *> &outputVarArrays)
{
    inputVarArrays.clear();
    outputVarArrays.clear();

    int rc = alloc_arrays(inputVarDims, inputVarArrays);
    if (rc < 0) return (rc);

    rc = alloc_arrays(outputVarDims, outputVarArrays);
    if (rc < 0) {
        free_arrays(inputVarArrays);
        return (rc);
    }

    return (0);
}

void grid2c(const vector<Grid *> &variables, const vector<float *> &inputVarArrays)
{
    assert(variables.size() == inputVarArrays.size());

    for (int i = 0; i < variables.size(); i++) {
        Grid::ConstIterator itr = variables[i]->cbegin();
        Grid::ConstIterator enditr = variables[i]->cend();

        float *bufptr = inputVarArrays[i];

        // Copy data. Missing values are always set to infinity for
        // easier Python handling
        //
        float mv = variables[i]->GetMissingValue();
        for (; itr != enditr; ++itr) {
            if (variables[i]->HasMissingData() && *itr == mv) {
                *bufptr = std::numeric_limits<double>::infinity();
            } else {
                *bufptr = *itr;
            }
            bufptr++;
        }
    }
}

void copy_region(const float *src, float *dst, const vector<size_t> &min, const vector<size_t> &max, const vector<size_t> &dims)
{
    vector<size_t> coord = min;
    for (size_t i = 0; i < VProduct(Dims(min, max)); i++) {
        size_t offset = LinearizeCoords(coord, dims);

        dst[i] = src[offset];

        coord = IncrementCoords(min, max, coord);
    }
}

};    // namespace

using namespace VAPoR;
using namespace Wasp;

int PyEngine::Initialize()
{
    int rc = MyPython::Instance()->Initialize();
    if (rc < 0) return (rc);

    // Secret numpy incantation required to use NumPy's C API. Hope this
    // crap works.
    //
    if (PyArray_API == NULL) { import_array1(-1) }

    return (0);
}

int PyEngine::AddFunction(string name, string script, const vector<string> &inputVarNames, const vector<string> &outputVarNames, const vector<string> &outputVarMeshes)
{
    assert(outputVarNames.size() == outputVarMeshes.size());

    // Output variables can't already be defined (either derived or native vars)
    //
    if (_checkOutVars(outputVarNames) < 0) return (-1);

    // Mesh name must be defined
    //
    if (_checkOutMeshes(outputVarMeshes) < 0) return (-1);

    const string timeCoordVarName = _getTimeCoordVarName(inputVarNames);

    vector<DerivedPythonVar *> dvars;
    for (int i = 0; i < outputVarNames.size(); i++) {
        string            vname = outputVarNames[i];
        DerivedPythonVar *dvar = new DerivedPythonVar(vname, "", DC::XType::FLOAT, outputVarMeshes[i], timeCoordVarName, true, inputVarNames, script, _dataMgr);
        dvars.push_back(dvar);

        if (dvar->Initialize() < 0) {
            for (int i = 0; i < dvars.size(); i++) delete dvars[i];
            SetErrMsg("Failed to initialized derived variable %s", vname.c_str());
            return (-1);
        }
        (void)_dataMgr->AddDerivedVar(dvar);
    }

    _functions[name] = func_c(name, script, inputVarNames, outputVarNames, outputVarMeshes, dvars);

    return (0);
}

void PyEngine::RemoveFunction(string name)
{
    map<string, func_c>::iterator itr = _functions.find(name);

    if (itr == _functions.end()) return;

    const func_c &                    func = itr->second;
    const vector<string> &            outputVarNames = func._outputVarNames;
    const vector<DerivedPythonVar *> &dvars = func._derivedVars;
    assert(outputVarNames.size() == dvars.size());

    for (int i = 0; i < outputVarNames.size(); i++) {
        _dataMgr->RemoveDerivedVar(outputVarNames[i]);
        if (dvars[i]) delete dvars[i];
    }

    _functions.erase(itr);
}

vector<string> PyEngine::GetFunctionNames() const
{
    map<string, func_c>::const_iterator itr;

    vector<string> names;
    for (itr = _functions.cbegin(); itr != _functions.cend(); ++itr) { names.push_back(itr->first); }

    return (names);
}

string PyEngine::GetFunctionScript(string name) const
{
    map<string, func_c>::const_iterator itr = _functions.find(name);

    if (itr == _functions.cend()) return ("");

    const func_c &func = itr->second;

    return (func._script);
}

PyEngine::~PyEngine()
{
    map<string, func_c>::iterator itr;
    while ((itr = _functions.begin()) != _functions.end()) { RemoveFunction(itr->first); }
}

int PyEngine::Calculate(const string &script, vector<string> inputVarNames, vector<vector<size_t>> inputVarDims, vector<float *> inputVarArrays, vector<string> outputVarNames,
                        vector<vector<size_t>> outputVarDims, vector<float *> outputVarArrays)
{
    assert(inputVarNames.size() == inputVarDims.size());
    assert(inputVarNames.size() == inputVarArrays.size());
    assert(outputVarNames.size() == outputVarDims.size());
    assert(outputVarNames.size() == outputVarArrays.size());

    // Convert the input arrays and put into dictionary:
    //
    PyObject *mainModule = PyImport_AddModule("__main__");
    if (!mainModule) {
        SetErrMsg("PyImport_AddModule(\"__main__\") : %s", MyPython::Instance()->PyErr().c_str());
        return -1;
    }

    PyObject *mainDict = PyModule_GetDict(mainModule);
    assert(mainDict != NULL);

    // Make a copy (needed for later cleanup)
    //
    PyObject *copyDict = PyDict_Copy(mainDict);
    assert(copyDict != NULL);

    // Copy arrays into python environment
    //
    int rc = _c2python(mainDict, inputVarNames, inputVarDims, inputVarArrays);
    if (rc < 0) {
        _cleanupDict(mainDict, copyDict);
        return (-1);
    }

    // Run the script
    //
    PyObject *retObj = PyRun_String(script.c_str(), Py_file_input, mainDict, mainDict);

    if (!retObj) {
        SetErrMsg("PyRun_String() : %s", MyPython::Instance()->PyErr().c_str());
        _cleanupDict(mainDict, copyDict);
        return -1;
    }

    // Retrieve calculated arrays from python environment
    //
    rc = _python2c(mainDict, outputVarNames, outputVarDims, outputVarArrays);
    if (rc < 0) {
        _cleanupDict(mainDict, copyDict);
        return (-1);
    }

    _cleanupDict(mainDict, copyDict);

    return (0);
}

void PyEngine::_cleanupDict(PyObject *mainDict, PyObject *copyDict)
{
    Py_ssize_t pos = 0;
    PyObject * key;
    PyObject * val;
    while (PyDict_Next(mainDict, &pos, &key, &val)) {
        if (PyDict_Contains(copyDict, key)) { continue; }
        PyObject_DelItem(mainDict, key);
    }
}

int PyEngine::_c2python(PyObject *dict, vector<string> inputVarNames, vector<vector<size_t>> inputVarDims, vector<float *> inputVarArrays)
{
    npy_intp pyDims[3];
    for (int i = 0; i < inputVarNames.size(); i++) {
        assert(inputVarDims[i].size() >= 1 && inputVarDims[i].size() <= 3);

        const vector<size_t> &dims = inputVarDims[i];
        for (int j = 0; j < dims.size(); j++) { pyDims[dims.size() - j - 1] = dims[j]; }

        PyObject *pyArray = PyArray_New(&PyArray_Type, dims.size(), pyDims, NPY_FLOAT32, NULL, (float *)inputVarArrays[i], 0, NPY_ARRAY_C_CONTIGUOUS | NPY_ARRAY_WRITEABLE, NULL);

        PyObject *ky = Py_BuildValue("s", inputVarNames[i].c_str());
        PyObject_SetItem(dict, ky, pyArray);
        Py_DECREF(pyArray);
    }

    return (0);
}

int PyEngine::_python2c(PyObject *dict, vector<string> outputVarNames, vector<vector<size_t>> outputVarDims, vector<float *> outputVarArrays)
{
    for (int i = 0; i < outputVarNames.size(); i++) {
        const string &vname = outputVarNames[i];

        PyObject *ky = Py_BuildValue("s", vname.c_str());

        PyObject *o = PyDict_GetItem(dict, ky);
        if (!o || !PyArray_CheckExact(o)) {
            SetErrMsg("Variable %s not produced by script", vname.c_str());
            return -1;
        }
        PyArrayObject *varArray = (PyArrayObject *)o;

        if (PyArray_TYPE(varArray) != NPY_FLOAT) {
            ;
            SetErrMsg("Variable %s data is not float32", vname.c_str());
            return -1;
        }

        const vector<size_t> &dims = outputVarDims[i];
        int                   nd = PyArray_NDIM(varArray);

        if (nd != dims.size()) {
            SetErrMsg("Shape of %s array does not match", vname.c_str());
            return -1;
        }

        npy_intp *pyDims = PyArray_DIMS(varArray);
        for (int j = 0; j < dims.size(); j++) {
            if (pyDims[dims.size() - j - 1] != dims[j]) {
                SetErrMsg("Shape of %s array does not match", vname.c_str());
                return -1;
            }
        }

        float *dataArray = (float *)PyArray_DATA(varArray);
        size_t nelements = VProduct(dims);

        float *outArray = outputVarArrays[i];
        for (size_t j = 0; j < nelements; j++) { outArray[j] = dataArray[j]; }
        Py_DECREF(varArray);
    }

    return (0);
}

PyEngine::DerivedPythonVar::DerivedPythonVar(string varName, string units, DC::XType type, string mesh, string time_coord_var, bool hasMissing, std::vector<string> inNames, string script,
                                             DataMgr *dataMgr)
: DerivedDataVar(varName), _varInfo(varName, units, type, "", std::vector<size_t>(), std::vector<bool>(), mesh, time_coord_var, DC::Mesh::NODE)
{
    _inNames = inNames;
    _script = script;
    _dataMgr = dataMgr;
    _dims.clear();
    _readSubsetFlag = false;
    if (hasMissing) {
        _varInfo.SetHasMissing(true);
        _varInfo.SetMissingValue(std::numeric_limits<double>::infinity());
    }
};

int PyEngine::DerivedPythonVar::Initialize()
{
    DC::Mesh m;
    bool     status = _dataMgr->GetMesh(_varInfo.GetMeshName(), m);
    if (!status) {
        SetErrMsg("Invalid mesh : %s", _varInfo.GetMeshName().c_str());
        return (-1);
    }

    vector<string> dimNames = m.GetDimNames();
    for (int i = 0; i < dimNames.size(); i++) {
        DC::Dimension dim;
        status = _dataMgr->GetDimension(dimNames[i], dim);
        assert(status);

        _dims.push_back(dim.GetLength());
    }

    string mesh = _varInfo.GetMeshName();

    // If all of the input variables and the output (derived) variable
    // are on the same mesh we can optimize by only calculating the
    // derived variable over the requested ROI (min and max extents)
    //
    _readSubsetFlag = true;
    for (int i = 0; i < _inNames.size(); i++) {
        DC::DataVar dvar;
        bool        ok = _dataMgr->GetDataVarInfo(_inNames[i], dvar);
        if (!ok || dvar.GetMeshName() != mesh) {
            _readSubsetFlag = false;
            break;
        }
    }

    return (0);
}

bool PyEngine::DerivedPythonVar::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _varInfo;
    return (true);
}

int PyEngine::DerivedPythonVar::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level = _dims;

    // No blocking
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

int PyEngine::DerivedPythonVar::OpenVariableRead(size_t ts, int level, int lod)
{
    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, level, lod);

    return (_fileTable.AddEntry(f));
}

int PyEngine::DerivedPythonVar::CloseVariable(int fd)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    _fileTable.RemoveEntry(fd);
    delete f;
    return (0);
}

int PyEngine::DerivedPythonVar::_readRegionAll(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    vector<Grid *> variables;

    size_t ts = f->GetTS();
    int    level = f->GetLevel();
    int    lod = f->GetLOD();

    int rc = DataMgrUtils::GetGrids(_dataMgr, ts, _inNames, false, &level, &lod, variables);
    if (rc < 0) return (-1);

    vector<vector<size_t>> inputVarDims;
    for (int i = 0; i < variables.size(); i++) { inputVarDims.push_back(variables[i]->GetDimensions()); }

    vector<size_t> dims, dummy;
    (void)GetDimLensAtLevel(level, dims, dummy);
    vector<vector<size_t>> outputVarDims = {dims};

    vector<float *> inputVarArrays;
    vector<float *> outputVarArrays;
    rc = alloc_arrays(inputVarDims, outputVarDims, inputVarArrays, outputVarArrays);
    if (rc < 0) {
        SetErrMsg("Error allocating  memory");
        DataMgrUtils::UnlockGrids(_dataMgr, variables);
        return (-1);
    }

    grid2c(variables, inputVarArrays);

    vector<string> outputVarNames = {_derivedVarName};
    rc = PyEngine::Calculate(_script, _inNames, inputVarDims, inputVarArrays, outputVarNames, outputVarDims, outputVarArrays);
    if (rc < 0) {
        DataMgrUtils::UnlockGrids(_dataMgr, variables);
        free_arrays(inputVarArrays, outputVarArrays);
        return (-1);
    }

    copy_region(outputVarArrays[0], region, min, max, outputVarDims[0]);

    DataMgrUtils::UnlockGrids(_dataMgr, variables);
    free_arrays(inputVarArrays, outputVarArrays);

    return (0);
}

int PyEngine::DerivedPythonVar::_readRegionSubset(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    vector<Grid *> variables;

    size_t ts = f->GetTS();
    int    level = f->GetLevel();
    int    lod = f->GetLOD();

    int rc = DataMgrUtils::GetGrids(_dataMgr, ts, _inNames, min, max, false, &level, &lod, variables);
    if (rc < 0) return (-1);

    vector<vector<size_t>> inputVarDims;
    for (int i = 0; i < variables.size(); i++) { inputVarDims.push_back(Dims(min, max)); }

    vector<vector<size_t>> outputVarDims = {Dims(min, max)};

    vector<float *> inputVarArrays;
    rc = alloc_arrays(inputVarDims, inputVarArrays);
    if (rc < 0) {
        SetErrMsg("Error allocating  memory");
        DataMgrUtils::UnlockGrids(_dataMgr, variables);
        return (-1);
    }

    grid2c(variables, inputVarArrays);

    vector<string>  outputVarNames = {_derivedVarName};
    vector<float *> outputVarArrays = {region};
    rc = PyEngine::Calculate(_script, _inNames, inputVarDims, inputVarArrays, outputVarNames, outputVarDims, outputVarArrays);
    if (rc < 0) {
        DataMgrUtils::UnlockGrids(_dataMgr, variables);
        free_arrays(inputVarArrays, outputVarArrays);
        return (-1);
    }

    DataMgrUtils::UnlockGrids(_dataMgr, variables);
    free_arrays(inputVarArrays);

    return (0);
}

int PyEngine::DerivedPythonVar::ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region)
{
    if (_readSubsetFlag) {
        return (_readRegionSubset(fd, min, max, region));
    } else {
        return (_readRegionAll(fd, min, max, region));
    }
}

bool PyEngine::DerivedPythonVar::VariableExists(size_t ts, int reflevel, int lod) const
{
    for (int i = 0; i < _inNames.size(); i++) {
        if (!_dataMgr->VariableExists(ts, _inNames[i], reflevel, lod)) { return (false); }
    }
    return (true);
}

bool PyEngine::DerivedPythonVar::GetDataVarInfo(DC::DataVar &cvar) const
{
    cvar = _varInfo;
    return (true);
}

bool PyEngine::_validOutputVar(string name) const
{
    vector<string> vars = _dataMgr->GetDataVarNames();

    if (find(vars.begin(), vars.end(), name) != vars.end()) return (false);

    vars = _dataMgr->GetCoordVarNames();

    if (find(vars.begin(), vars.end(), name) != vars.end()) return (false);

    return (true);
}

int PyEngine::_checkOutVars(const vector<string> &outputVarNames) const
{
    for (int i = 0; i < outputVarNames.size(); i++) {
        string vname = outputVarNames[i];
        if (!_validOutputVar(outputVarNames[i])) {
            SetErrMsg("Invalid derived variable name %s. Already in use.", vname.c_str());
            return (-1);
        }
    }
    return (0);
}

bool PyEngine::_validOutputMesh(string name) const
{
    vector<string> meshes = _dataMgr->GetMeshNames();

    return (find(meshes.begin(), meshes.end(), name) != meshes.end());
}

int PyEngine::_checkOutMeshes(const vector<string> &outputMeshNames) const
{
    for (int i = 0; i < outputMeshNames.size(); i++) {
        string name = outputMeshNames[i];
        if (!_validOutputMesh(outputMeshNames[i])) {
            SetErrMsg("Invalid derived variable mesh name %s", name.c_str());
            return (-1);
        }
    }

    return (0);
}

string PyEngine::_getTimeCoordVarName(const vector<string> &varNames) const
{
    string timeCoordVarName = _dataMgr->GetTimeCoordVarName();
    if (timeCoordVarName.empty()) return ("");

    for (int i = 0; i < varNames.size(); i++) {
        if (_dataMgr->IsTimeVarying(varNames[i])) return (timeCoordVarName);
    }

    return ("");
}
