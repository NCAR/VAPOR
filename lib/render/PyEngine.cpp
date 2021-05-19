#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <new>
#include <vapor/utils.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/PyEngine.h>
using namespace Wasp;
using namespace VAPoR;

#define NPY_NO_DEPRECATED_API NPY_API_VERSION
#include <numpy/arrayobject.h>

namespace {

struct varinfo_t {
    varinfo_t()
    {
        _g = NULL;
        _name.clear();
        _dims.clear();
        _coordNames.clear();
        _coordAxes.clear();
        _coordDims.clear();
    }

    Grid *                 _g;
    string                 _name;
    vector<size_t>         _dims;
    vector<string>         _coordNames;
    vector<int>            _coordAxes;
    vector<vector<size_t>> _coordDims;
};

void free_arrays(vector<float *> &arrays)
{
    for (int i = 0; i < arrays.size(); i++) {
        if (arrays[i]) { delete[] arrays[i]; }
    }
}

void free_arrays(vector<float *> &inputVarArrays, vector<float *> &outputVarArrays)
{
    free_arrays(inputVarArrays);
    free_arrays(outputVarArrays);
}

int alloc_arrays(const vector<vector<size_t>> &dimsVectors, vector<float *> &arrays)
{
    arrays.clear();

    for (int i = 0; i < dimsVectors.size(); i++) {
        size_t nElements = VProduct(dimsVectors[i]);

        float *buf = new (nothrow) float[nElements];
        if (!buf) {
            free_arrays(arrays);
            return (-1);
        }
        arrays.push_back(buf);
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

void copy_region(const float *src, float *dst, const vector<size_t> &min, const vector<size_t> &max, const vector<size_t> &dims)
{
    vector<size_t> coord = min;
    for (size_t i = 0; i < VProduct(Dims(min, max)); i++) {
        size_t offset = LinearizeCoords(coord, dims);

        dst[i] = src[offset];

        coord = IncrementCoords(min, max, coord);
    }
}

void copy_coord(const Grid *g, int axis, float *dst)
{
    vector<size_t> dims = g->GetCoordDimensions(axis);

    vector<size_t> min;
    vector<size_t> max;
    for (int i = 0; i < dims.size(); i++) {
        min.push_back(0);
        max.push_back(dims[i] - 1);
    }

    // Fetch user coordinates from grid. Note: assumes that if coordinate
    // dimensions are less than grid dimensions than the coordinate

    vector<size_t> index = min;
    vector<double> coord;
    for (size_t i = 0; i < VProduct(Dims(min, max)); i++) {
        g->GetUserCoordinates(index, coord);
        dst[i] = coord[axis];

        index = IncrementCoords(min, max, index);
    }
}

void grid2c(const vector<varinfo_t> &varInfoVec, const vector<float *> &inputVarArrays)
{
    int idx = 0;
    for (int i = 0; i < varInfoVec.size(); i++) {
        const varinfo_t &vref = varInfoVec[i];

        Grid *g = vref._g;

        Grid::ConstIterator itr = g->cbegin();
        Grid::ConstIterator enditr = g->cend();

        VAssert(idx < inputVarArrays.size());
        float *bufptr = inputVarArrays[idx];

        // Copy data. Missing values are always set to infinity for
        // easier Python handling
        //
        float mv = g->GetMissingValue();
        for (; itr != enditr; ++itr) {
            if (g->HasMissingData() && *itr == mv) {
                *bufptr = std::numeric_limits<double>::infinity();
            } else {
                *bufptr = *itr;
            }
            bufptr++;
        }

        idx++;
        for (int j = 0; j < vref._coordNames.size(); j++) {
            VAssert(idx < inputVarArrays.size());
            float *bufptr = inputVarArrays[idx];

            copy_coord(g, vref._coordAxes[j], bufptr);

            idx++;
        }
    }
}

void get_var_info(DataMgr *dataMgr, const vector<Grid *> &gs, const vector<string> &varNames, bool coordFlag, vector<varinfo_t> &varinfoVec)
{
    varinfoVec.clear();

    VAssert(gs.size() == varNames.size());

    // Need to keep track of all coordinate variables. We generate a
    // unique list
    //
    vector<string> allCoordVars;

    // Build a vector of grids and *unique* coordinate variables
    //
    for (int i = 0; i < gs.size(); i++) {
        varinfo_t varinfo;
        varinfo._g = gs[i];
        varinfo._name = varNames[i];
        varinfo._dims = gs[i]->GetDimensions();

        varinfo._coordNames.clear();
        varinfo._coordAxes.clear();
        varinfo._coordDims.clear();

        if (!coordFlag) {
            varinfoVec.push_back(varinfo);
            continue;
        }

        DC::DataVar dvar;
        bool        ok = dataMgr->GetDataVarInfo(varNames[i], dvar);
        VAssert(ok);

        DC::Mesh m;
        ok = dataMgr->GetMesh(dvar.GetMeshName(), m);
        VAssert(ok);

        vector<string> coordNames = m.GetCoordVars();
        for (int j = 0; j < coordNames.size(); j++) {
            vector<string>::iterator itr;
            itr = find(allCoordVars.begin(), allCoordVars.end(), coordNames[j]);

            if (itr != allCoordVars.end()) continue;

            DC::CoordVar cvar;
            ok = dataMgr->GetCoordVarInfo(coordNames[j], cvar);
            VAssert(ok);

            varinfo._coordNames.push_back(coordNames[j]);
            varinfo._coordAxes.push_back(cvar.GetAxis());
            varinfo._coordDims.push_back(gs[i]->GetCoordDimensions(cvar.GetAxis()));
        }
        varinfoVec.push_back(varinfo);
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

int PyEngine::AddFunction(string name, string script, const vector<string> &inputVarNames, const vector<string> &outputVarNames, const vector<string> &outputVarMeshes, bool coordFlag)
{
    VAssert(outputVarNames.size() == outputVarMeshes.size());

    // cout << "PyEngine::AddFunction() " << name << endl;
    // cout << "	" << script << endl;

    // No-op if not defined
    //
    RemoveFunction(name);

    // Output variables can't already be defined (either derived or native vars)
    //
    if (_checkOutVars(outputVarNames) < 0) return (-1);

    // Mesh name must be defined
    //
    if (_checkOutMeshes(outputVarMeshes) < 0) return (-1);

    // Test the script syntatical correctness. Only way to do this
    // with crappy Python API is to compile the string to an
    // object :-(
    //
    PyObject *retObj = Py_CompileString(script.c_str(), "", Py_file_input);
    if (!retObj) {
        SetErrMsg("Py_CompileString() : %s", MyPython::Instance()->PyErr().c_str());
        return -1;
    }
    Py_DECREF(retObj);

    const string timeCoordVarName = _getTimeCoordVarName(inputVarNames);

    vector<DerivedPythonVar *> dvars;
    for (int i = 0; i < outputVarNames.size(); i++) {
        string            vname = outputVarNames[i];
        DerivedPythonVar *dvar = new DerivedPythonVar(vname, "", DC::XType::FLOAT, outputVarMeshes[i], timeCoordVarName, true, inputVarNames, script, _dataMgr, coordFlag);
        dvars.push_back(dvar);

        if (dvar->Initialize() < 0) {
            for (int i = 0; i < dvars.size(); i++) delete dvars[i];
            SetErrMsg("Failed to initialized derived variable %s", vname.c_str());
            return (-1);
        }
        (void)_dataMgr->AddDerivedVar(dvar);
    }

    _functions[name] = func_c(name, script, inputVarNames, outputVarNames, outputVarMeshes, dvars, coordFlag);

    return (0);
}

void PyEngine::RemoveFunction(string name)
{
    map<string, func_c>::iterator itr = _functions.find(name);

    if (itr == _functions.end()) return;

    const func_c &                    func = itr->second;
    const vector<string> &            outputVarNames = func._outputVarNames;
    const vector<DerivedPythonVar *> &dvars = func._derivedVars;
    VAssert(outputVarNames.size() == dvars.size());

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

bool PyEngine::GetFunctionScript(string name, string &script, std::vector<string> &inputVarNames, std::vector<string> &outputVarNames, std::vector<string> &outputMeshNames, bool &coordFlag) const
{
    script.clear();
    inputVarNames.clear();
    outputVarNames.clear();
    outputMeshNames.clear();

    map<string, func_c>::const_iterator itr = _functions.find(name);

    if (itr == _functions.cend()) return (false);

    const func_c &func = itr->second;

    script = func._script;
    inputVarNames = func._inputVarNames;
    outputVarNames = func._outputVarNames;
    outputMeshNames = func._outputMeshNames;
    coordFlag = func._coordFlag;

    return (true);
}

string PyEngine::GetFunctionStdout(string name) const
{
    map<string, func_c>::const_iterator itr = _functions.find(name);

    if (itr == _functions.cend()) return ("");
    const func_c &func = itr->second;

    string s;
    for (int i = 0; i < func._derivedVars.size(); i++) { s += func._derivedVars[i]->GetScriptStdout(); }
    return (s);
}

PyEngine::~PyEngine()
{
    map<string, func_c>::iterator itr;
    while ((itr = _functions.begin()) != _functions.end()) { RemoveFunction(itr->first); }
}

int PyEngine::Calculate(const string &script, vector<string> inputVarNames, vector<vector<size_t>> inputVarDims, vector<float *> inputVarArrays, vector<string> outputVarNames,
                        vector<vector<size_t>> outputVarDims, vector<float *> outputVarArrays)
{
    VAssert(inputVarNames.size() == inputVarDims.size());
    VAssert(inputVarNames.size() == inputVarArrays.size());
    VAssert(outputVarNames.size() == outputVarDims.size());
    VAssert(outputVarNames.size() == outputVarArrays.size());

    vector<string> allNames = inputVarNames;
    allNames.insert(allNames.end(), outputVarNames.begin(), outputVarNames.end());

    // cout << "PyEngine::Calculate() " << script << endl;

    // Convert the input arrays and put into dictionary:
    //
    PyObject *mainModule = PyImport_AddModule("__main__");
    if (!mainModule) {
        SetErrMsg("PyImport_AddModule(\"__main__\") : %s", MyPython::Instance()->PyErr().c_str());
        return -1;
    }

    PyObject *mainDict = PyModule_GetDict(mainModule);
    VAssert(mainDict != NULL);

    // Copy arrays into python environment
    //
    int rc = _c2python(mainDict, inputVarNames, inputVarDims, inputVarArrays);
    if (rc < 0) {
        _cleanupDict(mainDict, inputVarNames);
        return (-1);
    }

    // Run the script
    //
    PyObject *retObj = PyRun_String(script.c_str(), Py_file_input, mainDict, mainDict);

    if (!retObj) {
        SetErrMsg("PyRun_String() : %s", MyPython::Instance()->PyErr().c_str());
        _cleanupDict(mainDict, inputVarNames);
        return -1;
    }

    // Retrieve calculated arrays from python environment
    //
    rc = _python2c(mainDict, outputVarNames, outputVarDims, outputVarArrays);
    if (rc < 0) {
        _cleanupDict(mainDict, allNames);
        return (-1);
    }

    _cleanupDict(mainDict, allNames);

    return (0);
}

void PyEngine::_cleanupDict(PyObject *mainDict, vector<string> keynames)
{
    for (int i = 0; i < keynames.size(); i++) {
        PyObject *key = PyUnicode_FromString(keynames[i].c_str());
        if (!key) continue;
        if (PyDict_Contains(mainDict, key)) { PyObject_DelItem(mainDict, key); }
        Py_DECREF(key);
    }
}

int PyEngine::_c2python(PyObject *dict, vector<string> inputVarNames, vector<vector<size_t>> inputVarDims, vector<float *> inputVarArrays)
{
    npy_intp pyDims[3];
    for (int i = 0; i < inputVarNames.size(); i++) {
        VAssert(inputVarDims[i].size() >= 1 && inputVarDims[i].size() <= 3);

        const vector<size_t> &dims = inputVarDims[i];
        for (int j = 0; j < dims.size(); j++) { pyDims[dims.size() - j - 1] = dims[j]; }

        PyObject *pyArray = PyArray_SimpleNewFromData(dims.size(), pyDims, NPY_FLOAT32, inputVarArrays[i]);

        PyObject *ky = Py_BuildValue("s", inputVarNames[i].c_str());
        PyObject_SetItem(dict, ky, pyArray);
        Py_DECREF(ky);
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
    }

    return (0);
}

PyEngine::DerivedPythonVar::DerivedPythonVar(string varName, string units, DC::XType type, string mesh, string time_coord_var, bool hasMissing, std::vector<string> inNames, string script,
                                             DataMgr *dataMgr, bool coordFlag)
: DerivedDataVar(varName), _varInfo(varName, units, type, "", std::vector<size_t>(), std::vector<bool>(), mesh, time_coord_var, DC::Mesh::NODE)
{
    _inNames = inNames;
    _script = script;
    _dataMgr = dataMgr;
    _coordFlag = coordFlag;
    _dims.clear();
    _meshMatchFlag = false;
    _stdoutString.clear();
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
        status = _dataMgr->GetDimension(dimNames[i], dim, -1);
        VAssert(status);

        _dims.push_back(dim.GetLength());
    }

    string mesh = _varInfo.GetMeshName();

    // If all of the input variables and the output (derived) variable
    // are on the same mesh we can optimize by only calculating the
    // derived variable over the requested ROI (min and max extents)
    //
    _meshMatchFlag = true;
    for (int i = 0; i < _inNames.size(); i++) {
        DC::DataVar dvar;
        bool        ok = _dataMgr->GetDataVarInfo(_inNames[i], dvar);
        if (!ok || dvar.GetMeshName() != mesh) {
            _meshMatchFlag = false;
            break;
        }
    }

    if (_meshMatchFlag && _inNames.size()) {
        DC::DataVar dvar;
        bool        ok = _dataMgr->GetDataVarInfo(_inNames[0], dvar);
        VAssert(ok);
        _varInfo.SetCRatios(dvar.GetCRatios());
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
    if (_meshMatchFlag && _inNames.size()) {
        int rc = _dataMgr->GetDimLensAtLevel(_inNames[0], level, dims_at_level, -1);
        VAssert(rc >= 0);
    } else {
        dims_at_level = _dims;
    }

    // No blocking
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

size_t PyEngine::DerivedPythonVar::GetNumRefLevels() const
{
    if (_meshMatchFlag && _inNames.size()) { return (_dataMgr->GetNumRefLevels(_inNames[0])); }

    return (1);
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

    vector<varinfo_t> varInfoVec;
    get_var_info(_dataMgr, variables, _inNames, _coordFlag, varInfoVec);

    vector<vector<size_t>> inputVarDims;
    vector<string>         inputNames;
    for (int i = 0; i < varInfoVec.size(); i++) {
        const varinfo_t &vref = varInfoVec[i];

        inputNames.push_back(vref._name);
        inputVarDims.push_back(vref._dims);
        for (int j = 0; j < vref._coordNames.size(); j++) {
            inputNames.push_back(vref._coordNames[j]);
            inputVarDims.push_back(vref._coordDims[j]);
        }
    }

    vector<size_t> dims, dummy;
    (void)GetDimLensAtLevel(level, dims, dummy);
    vector<vector<size_t>> outputVarDims = {dims};

    vector<float *> inputVarArrays;
    vector<float *> outputVarArrays;
    rc = alloc_arrays(inputVarDims, outputVarDims, inputVarArrays, outputVarArrays);
    if (rc < 0) {
        SetErrMsg("Error allocating  memory");
        return (-1);
    }

    grid2c(varInfoVec, inputVarArrays);

    //
    // clear stdout from static class
    //
    (void)MyPython::Instance()->PyOut();

    vector<string> outputVarNames = {_derivedVarName};
    rc = PyEngine::Calculate(_script, inputNames, inputVarDims, inputVarArrays, outputVarNames, outputVarDims, outputVarArrays);

    //
    // Capture any stdout
    //
    _stdoutString = MyPython::Instance()->PyOut().c_str();

    if (rc < 0) {
        free_arrays(inputVarArrays, outputVarArrays);
        return (-1);
    }

    copy_region(outputVarArrays[0], region, min, max, outputVarDims[0]);

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

    vector<varinfo_t> varInfoVec;
    get_var_info(_dataMgr, variables, _inNames, _coordFlag, varInfoVec);

    vector<vector<size_t>> inputVarDims;
    vector<string>         inputNames;
    for (int i = 0; i < varInfoVec.size(); i++) {
        const varinfo_t &vref = varInfoVec[i];

        inputNames.push_back(vref._name);
        inputVarDims.push_back(vref._dims);
        for (int j = 0; j < vref._coordNames.size(); j++) {
            inputNames.push_back(vref._coordNames[j]);
            inputVarDims.push_back(vref._coordDims[j]);
        }
    }

    // output and input variable(s) (if they exist) are all defined
    // on the same mesh (they have same dimensions)
    //
    vector<vector<size_t>> outputVarDims;
    vector<size_t>         minAbs(min.size(), 0);
    if (inputVarDims.size()) {
        outputVarDims.push_back(inputVarDims[0]);
        minAbs = variables[0]->GetMinAbs();
    } else {
        outputVarDims.push_back(Dims(min, max));
    }

    vector<float *> inputVarArrays;
    vector<float *> outputVarArrays;
    rc = alloc_arrays(inputVarDims, outputVarDims, inputVarArrays, outputVarArrays);
    if (rc < 0) {
        SetErrMsg("Error allocating  memory");
        return (-1);
    }

    grid2c(varInfoVec, inputVarArrays);

    //
    // clear stdout from static class
    //
    (void)MyPython::Instance()->PyOut();

    vector<string> outputVarNames = {_derivedVarName};
    rc = PyEngine::Calculate(_script, inputNames, inputVarDims, inputVarArrays, outputVarNames, outputVarDims, outputVarArrays);

    //
    // Capture any stdout
    //
    _stdoutString = MyPython::Instance()->PyOut().c_str();

    if (rc < 0) {
        free_arrays(inputVarArrays, outputVarArrays);
        return (-1);
    }

    // The min and max coordinates input to this method are relative to
    // the entire domain. We need to correct them by substracting off the
    // origin of the ROI contained in the Grid objects
    //
    vector<size_t> min_roi, max_roi;
    for (int i = 0; i < min.size(); i++) {
        min_roi.push_back(min[i] - minAbs[i]);
        max_roi.push_back(max[i] - minAbs[i]);
    }
    copy_region(outputVarArrays[0], region, min_roi, max_roi, outputVarDims[0]);

    free_arrays(inputVarArrays, outputVarArrays);

    return (0);
}

int PyEngine::DerivedPythonVar::ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region)
{
    if (_meshMatchFlag) {
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
