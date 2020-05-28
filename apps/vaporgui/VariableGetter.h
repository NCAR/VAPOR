#pragma once

#include "Flags.h"

//! \class VariableGetter
//! A class that takes flags that indicate variable type on construction,
//! and looks up the currently active variable for that type.

namespace {
    std::string getCurrentVariable(
        VAPoR::RenderParams* params, 
        VAPoR::DataMgr*      dataMgr,
        VariableFlags        variableFlags 
    ) {
        string varName;
        if ( variableFlags & SCALAR ) {
            varName = params->GetVariableName();
        }
        else if ( variableFlags & VECTOR ) {
            vector <string> varNames = params->GetFieldVariableNames();
            if( varNames.size() > 0 )
            {
                varName = varNames[0];
                size_t vardim;
                for( int i = 0; i < varNames.size(); i++ )
                {
                    vardim = dataMgr->GetNumDimensions( varNames[i]);
                    if( vardim == 3 )
                    {
                        varName = varNames[i];
                        break;
                    }
                }
            }
        }
        else if ( variableFlags & HEIGHT ) {
            varName = params->GetHeightVariableName();
        }
        else if ( variableFlags & AUXILIARY ) {
            vector<string> varNames = params->GetAuxVariableNames();
            if(varNames.size() > 0)
            {
                varName = varNames[0];
                size_t vardim;
                for( int i = 0; i < varNames.size(); i++ )
                {
                    vardim = dataMgr->GetNumDimensions( varNames[i]);
                    if( vardim == 3 )
                    {
                        varName = varNames[i];
                        break;
                    }
                }
            }
        }
        else if ( variableFlags & COLOR ) {
            varName = params->GetColorMapVariableName();
        }

        return varName;
    }
}
