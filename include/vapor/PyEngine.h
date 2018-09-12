#include <vector>
#include <Python.h>
#include <vapor/DataMgr.h>
#include <vapor/DC.h>

#pragma once

namespace VAPoR {

class VDF_API PyEngine : public Wasp::MyBase {
public:
    PyEngine() {}
    ~PyEngine(){};

    int Initialize();

    int AddFunction(string name, const vector<string> &inputs, const vector<string> &outputs, const vector<string> &outMeshes);

    void RemoveFunction(string name);

    int Calculate(const string &script, vector<string> inputVarNames, vector<vector<size_t>> inputVarDims, vector<float *> inputVarArrays, vector<string> outputVarNames,
                  vector<vector<size_t>> outputVarDims, vector<float *> outputVarArrays) const;

private:
    class VDF_API DerivedPythonVar : public DerivedDataVar {
    public:
        DerivedPythonVar(string varName, string mesh, std::vector<string> inNames, string script, DataMgr *dataMgr) : DerivedDataVar(varName)
        {
            _mesh = mesh;
            _inNames = inNames;
            _script = script;
            _dataMgr = dataMgr;
        };

        ~DerivedPythonVar() {}

        int Initialize();

        bool GetBaseVarInfo(DC::BaseVar &var) const;

        std::vector<string> GetInputs() { return (_inNames); }

        virtual int GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level);

        int OpenVariableRead(size_t ts, int level = 0, int lod = 0);

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
    };

    void _cleanupDict(PyObject *mainDict, PyObject *copyDict) const;

    int _c2python(PyObject *dict, vector<string> inputVarNames, vector<vector<size_t>> inputVarDims, vector<float *> inputVarArrays) const;

    int _python2c(PyObject *dict, vector<string> outputVarNames, vector<vector<size_t>> outputVarDims, vector<float *> outputVarArrays) const;
};
};    // namespace VAPoR
