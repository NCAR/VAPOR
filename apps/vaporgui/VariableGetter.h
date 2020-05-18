#pragma once

#include "Flags.h"

//! \class VariableGetter
//! A class that takes flags that indicate variable type on construction,
//! and looks up the currently active variable for that type.

class VariableGetter {

public:
    VariableGetter( 
        VAPoR::RenderParams* params, 
        VAPoR::DataMgr*      dataMgr,
        VariableFlags        variableFlags 
    ) {
        _variableFlags = variableFlags;
        _dataMgr = dataMgr;
        _rParams = params;
    }

    std::string getCurrentVariable() const {
        string varName;
        if (_variableFlags & SCALAR) {
            varName = _rParams->GetVariableName();
        }
        else if (_variableFlags & VECTOR) {
            vector <string> varNames = _rParams->GetFieldVariableNames();
            if( varNames.size() > 0 )
            {
                varName = varNames[0];
                size_t vardim;
                for( int i = 0; i < varNames.size(); i++ )
                {
                    vardim = _dataMgr->GetNumDimensions( varNames[i]);
                    if( vardim == 3 )
                    {
                        varName = varNames[i];
                        break;
                    }
                }
            }
        }
        else if (_variableFlags & HEIGHT) {
            varName = _rParams->GetHeightVariableName();
        }
        else if (_variableFlags & AUXILIARY) {
            vector<string> varNames = _rParams->GetAuxVariableNames();
            if(varNames.size() > 0)
            {
                varName = varNames[0];
                size_t vardim;
                for( int i = 0; i < varNames.size(); i++ )
                {
                    vardim = _dataMgr->GetNumDimensions( varNames[i]);
                    if( vardim == 3 )
                    {
                        varName = varNames[i];
                        break;
                    }
                }
            }
        }
        else if (_variableFlags & COLOR) {
            varName = _rParams->GetColorMapVariableName();
        }

        //if (varName.empty()) {
        //    setEnabled(false);
        //    return "";
        //}
        return varName;
    }

private:
    VAPoR::RenderParams* _rParams;
    VAPoR::DataMgr*      _dataMgr;
    VariableFlags        _variableFlags;

};
