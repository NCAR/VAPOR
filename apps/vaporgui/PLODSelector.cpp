#include <sstream>
#include <assert.h>

#include <VComboBox.h>
#include <vapor/ParamsBase.h>
#include <vapor/RenderParams.h>

#include "PLODSelector.h"
#include "VLineItem.h"
#include "VariableGetter.h"

using VAPoR::RenderParams;
using VAPoR::Box;

PLODSelector::PLODSelector()
: PEnumDropdown("", {}, {}, "Level of Detail (HLI)")
{}

void PLODSelector::Reinit( VariableFlags varFlags ) {
    _variableFlags = varFlags;
}   

// Override PEnumDropdown::dropdownIndexChanged to call
// RenderParams setter directly, instead of PWidget::setParamsLong.
// PWidget::setParamsLong depends on a tag, which we are not using
void PLODSelector::dropdownIndexChanged( int index ) {
    int value;
    if (_enumMap.empty()) {
        value = index;
    } else {
        VAssert(index >= 0 && index < _enumMap.size());
        value = _enumMap[index];
    }

    setParamsLong( (long)index );
}

void PLODSelector::updateGUI() const
{
    RenderParams *rParams = dynamic_cast<RenderParams*>(getParams());
    assert(rParams && "Params must be RenderParams");
    static_cast<void>(rParams);        // Silence unused variable warning

    //VariableGetter varGetter( rParams, _dataMgr, _variableFlags );
    //std::string varName = varGetter.getCurrentVariable();
    std::string varName = getCurrentVariable( 
        rParams, 
        _dataMgr, 
        _variableFlags 
    );

    std::vector <long> lodCFs;
    std::vector <std::string> lodStrs;

    vector <size_t> cratios = _dataMgr->GetCRatios(varName);

    for (int i=0; i<cratios.size(); i++) {
        ostringstream oss;
        lodCFs.push_back((float) 1.0 / cratios[i]);

            oss << i << " (" << cratios[i] << ":1)";
            lodStrs.push_back(oss.str());
    }

    int lodReq = rParams->GetCompressionLevel();
    int lod = lodReq < 0 ? 0 : lodReq;
    lod = lodReq >= lodCFs.size() ? lodCFs.size()-1 : lodReq;

    _vComboBox->SetOptions( lodStrs );
    _vComboBox->SetIndex( lodReq );
}
