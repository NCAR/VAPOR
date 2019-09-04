#pragma once

#include <vector>
#include <vapor/DataMgr.h>

namespace VAPoR {

class DataStatus;
class ParamsMgr;
class PyEngine;

//! \class CalcEngineMgr
//! \brief A class for managing CalcEngine class instances
//! \author John Clyne
//
class RENDER_API CalcEngineMgr : public Wasp::MyBase {
public:
    //! Constructor for CalcEngineMgr class
    //!
    //! \param[in] dataMgr A pointer to a DataMgr instance upon which derived
    //! variables created by this class will be managed.
    //
    CalcEngineMgr(DataStatus *dataStatus, ParamsMgr *paramsMgr)
    {
        VAssert(dataStatus != NULL);
        VAssert(paramsMgr != NULL);

        _dataStatus = dataStatus;
        _paramsMgr = paramsMgr;
    }

    ~CalcEngineMgr();

    int AddFunction(string scriptType, string dataSetName, string scriptName, string script, const vector<string> &inputVarNames, const vector<string> &outputVarNames,
                    const vector<string> &outputVarMeshes, bool coordFlag = false);

    void RemoveFunction(string scriptType, string dataSetName, string scriptName);

    bool GetFunctionScript(string scriptType, string datasetName, string scriptName, string &script, vector<string> &inputVarNames, vector<string> &outputVarNames, vector<string> &outputVarMeshes,
                           bool &coordFlag);

    string GetFunctionStdout(string scriptType, string dataSetName, string scriptName);

    std::vector<string> GetFunctionNames(string scriptType, string datasetName);

    //! Rebuild from params database
    //!
    //! When invoked this method rebuilds internal state using the ParamsMgr
    //! \p paramsMgr passed in to the constructor
    //
    void ReinitFromState() { _sync(); }

private:
    const DataStatus *_dataStatus;
    const ParamsMgr * _paramsMgr;

    CalcEngineMgr() {}
    void _sync();
    void _clean();

    std::map<string, PyEngine *> _pyScripts;
};
};    // namespace VAPoR
