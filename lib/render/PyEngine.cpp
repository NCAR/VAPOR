#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <vapor/MyPython.h>
#include <vapor/PyEngine.h>
using namespace Wasp;
using namespace VAPoR;

#define NPY_NO_DEPRECATED_API NPY_API_VERSION
#include <numpy/arrayobject.h>

namespace {
// Product of elements in a vector
//
size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}

};    // namespace

using namespace VAPoR;
using namespace Wasp;

// PyEngine::PyEngine(DataMgr *dataMgr) {
//	_dataMgr = dataMgr;
//}

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

int PyEngine::AddFunction(string name, const vector<string> &inputVarNames, const vector<string> &outputVarNames, const vector<string> &outputVarMeshes) { return (0); }

void PyEngine::RemoveFunction(string name) {}

bool PyEngine::TestFunction(string name, string &stdoutMsg, string &stderrMsg) {}

int PyEngine::Calculate(const string &script, vector<string> inputVarNames, vector<vector<size_t>> inputVarDims, vector<float *> inputVarArrays, vector<string> outputVarNames,
                        vector<vector<size_t>> outputVarDims, vector<float *> outputVarArrays) const
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

void PyEngine::_cleanupDict(PyObject *mainDict, PyObject *copyDict) const
{
    Py_ssize_t pos = 0;
    PyObject * key;
    PyObject * val;
    while (PyDict_Next(mainDict, &pos, &key, &val)) {
        if (PyDict_Contains(copyDict, key)) { continue; }
        PyObject_DelItem(mainDict, key);
    }
}

int PyEngine::_c2python(PyObject *dict, vector<string> inputVarNames, vector<vector<size_t>> inputVarDims, vector<float *> inputVarArrays) const
{
    npy_intp pyDims[3];
    for (int i = 0; i < inputVarNames.size(); i++) {
        assert(inputVarDims[i].size() >= 1 && inputVarDims[i].size() <= 3);

        const vector<size_t> &dims = inputVarDims[i];
        for (int j = 0; j < dims.size(); j++) { pyDims[dims.size() - j - 1] = dims[j]; }

        PyObject *pyArray = PyArray_New(&PyArray_Type, dims.size(), pyDims, NPY_FLOAT32, NULL, inputVarArrays[i], 0, NPY_ARRAY_C_CONTIGUOUS | NPY_ARRAY_WRITEABLE, NULL);

        PyObject *ky = Py_BuildValue("s", inputVarNames[i].c_str());
        PyObject_SetItem(dict, ky, pyArray);
        Py_DECREF(pyArray);
    }

    return (0);
}

int PyEngine::_python2c(PyObject *dict, vector<string> outputVarNames, vector<vector<size_t>> outputVarDims, vector<float *> outputVarArrays) const
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
        size_t nelements = vproduct(dims);

        float *outArray = outputVarArrays[i];
        for (size_t j = 0; j < nelements; j++) { outArray[j] = dataArray[j]; }
        Py_DECREF(varArray);
    }

    return (0);
}

DerivedPythonVar::DerivedPythonVar(string varName, string units, XType type string mesh, string time_coord_var, bool hasMissing std::vector<string> inNames, string script, DataMgr *dataMgr)
: DerivedDataVar(varName), _varInfo(varName, units, type, "", std::vector<size_t>(), std::vector<bool>(), mesh, time_cord_var, location)
{
    _script = script;
    _dataMgr = dataMgr;
    if (hasMissing) {
        _varInfo.SetHasMissing(true);
        _varInfo.SetMissingValue(std::numeric_limits<double>::infinity());
    }
};

~DerivedPythonVar::DerivedPythonVar() {}

int DerivedPythonVar::Initialize()
{
    DC::Mesh m;
    bool     status = _dataMgr->GetMesh(_varInfo.GetMeshName(), m);
    if (!status) {
        SetErrMsg("Invalid mesh : %s" _varInfo.GetMeshName().c_str());
        return (-1);
    }

    vector<string> dimNames = m.GetDimNames();
    for (int i = 0; i < dimNames.size(); i++) {
        DC::Dimension dim;
        status = _dataMgr->GetDimension(dimNames[i], dim);
        assert(status);

        _dims.push_back(dim.GetLength());
    }
    return (0);
}

bool GetBaseVarInfo(DC::BaseVar &var) const { var = _varInfo; }

std::vector<string> GetInputs() { return (_inNames); }

virtual int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level)
{
    dims_at_level = _dims;
    bs_at_level = _dims;
    return (0);
}

int OpenVariableRead(size_t ts, int, int);

int CloseVariable(int fd);

int ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

int ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region);

bool VariableExists(size_t ts, int reflevel, int lod) const;

bool GetDataVarInfo(DC::DataVar &cvar);

private:
string              _mesh;
std::vector<string> _inNames;
string              _script;
DataMgr *           _dataMgr;
DC::FileTable       _fileTable;
}
;
