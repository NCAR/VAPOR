#pragma once

#include "PEnumDropdown.h"
#include "Flags.h"

//! \class PRefinementSelector
//! Allows the user to select Level of Detail (LOD). 
//!
//! User options are generated strings showing compression ratios,
//! which are mapped to an enum for setting RenderParams.

class PRefinementSelector : public PEnumDropdown {
    Q_OBJECT
    
public:
    
    //PRefinementSelector(const std::string &tag, const std::string &label="");
    PRefinementSelector();
    void Reinit( VariableFlags varFlags );

protected:
    void updateGUI() const override;
    bool requireDataMgr() const override { return true; }

    VariableFlags _varFlags;
    void getCmpFactors( 
        std::string varName, 
        std::vector<long> refinements, 
        std::vector<std::string> refinementStrings
    );

protected slots:
    void dropdownIndexChanged( int index ) override;

private:
    VariableFlags _variableFlags;
};
