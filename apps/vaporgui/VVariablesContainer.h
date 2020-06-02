#pragma once

#include "VContainer.h"
#include "Flags.h"

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}

class VVariablesSection;
class VFidelitySection;

//! class VVariablesContainer
//! A VContainer that holds pointers to a VVariablesSection, 
//! and a VFidelitySection.  Updates, and reinits these pointers
//! as necessary.

class VVariablesContainer : public VContainer {

    Q_OBJECT

public:

    VVariablesContainer();

    void Reinit(
        VariableFlags varFlags,
        DimFlags      dimFlags
    );

    void Update(
        VAPoR::RenderParams* rParams,
        VAPoR::ParamsMgr*    paramsMgr,
        VAPoR::DataMgr*      dataMgr
    );

private:
    VVariablesSection* _variablesSection;
    VFidelitySection*  _fidelitySection;
};
