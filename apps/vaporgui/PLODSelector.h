#pragma once

#include "PEnumDropdown.h"
#include "Flags.h"

//! \class PLODSelector
//! Allows the user to select Level of Detail (LOD). 
//!
//! User options are generated strings showing compression ratios,
//! which are mapped to an enum for setting RenderParams.

class PLODSelector : public PEnumDropdown {
    Q_OBJECT
    
public:
    
    PLODSelector();
    void Reinit( VariableFlags varFlags );

protected:
    void updateGUI() const override;
    bool requireDataMgr() const override { return true; }

    VariableFlags _varFlags;
    void getCmpFactors( 
        std::string varName, 
        std::vector<long> lodCFs, 
        std::vector<std::string> lodStrs
    );

protected slots:
    void dropdownIndexChanged( int index ) override;

private:
    VariableFlags _variableFlags;
};
