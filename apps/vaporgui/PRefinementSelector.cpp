#include <sstream>
#include <assert.h>

#include <VComboBox.h>
#include <vapor/RenderParams.h>

#include "PRefinementSelector.h"
#include "VariableGetter.h"

PRefinementSelector::PRefinementSelector()
: PEnumDropdown("", {}, {}, "Refinement level")
{}

void PRefinementSelector::Reinit( VariableFlags varFlags ) {
    _variableFlags = varFlags;
}   

// Override PEnumDropdown::dropdownIndexChanged to call
// RenderParams setter directly, instead of PWidget::setParamsLong.
// PWidget::setParamsLong depends on a tag, which we are not using
void PRefinementSelector::dropdownIndexChanged( int index ) {
    int value;
    if (_enumMap.empty()) {
        value = index;
    } else {
        VAssert(index >= 0 && index < _enumMap.size());
        value = _enumMap[index];
    }

    VAPoR::ParamsBase* params = getParams();
    VAPoR::RenderParams* rParams = dynamic_cast<VAPoR::RenderParams*>( params );
    //VAPoR::RenderParams* rParams = dynamic_cast<VAPoR::RenderParams*>( _params );
    rParams->SetRefinementLevel(value);
}

void PRefinementSelector::updateGUI() const {
    VAPoR::RenderParams *rParams = dynamic_cast<VAPoR::RenderParams*>(getParams());
    assert(rParams && "Params must be RenderParams");
    static_cast<void>(rParams);        // Silence unused variable warning

    VAPoR::DataMgr* dataMgr = getDataMgr();
    std::string varName = getCurrentVariable( rParams, dataMgr, _variableFlags );

    vector <long> multiresCFs;
    vector <string> multiresStrs;
    
    int numLevels = dataMgr->GetNumRefLevels(varName);
    vector <size_t> nGridPts;
    for (int l=0; l<numLevels; l++) {

        vector <size_t> dims_at_level;
        int rc = dataMgr->GetDimLensAtLevel(varName, l, dims_at_level);
        VAssert(rc >= 0);

        size_t n = 1;
        ostringstream oss;
        oss << l << " (";
        for (int j=0; j<dims_at_level.size(); j++) {
            n *= dims_at_level[j];

            oss << dims_at_level[j];
            if (j < dims_at_level.size()-1) oss << "x";
        }
        nGridPts.push_back(n);

        oss << ")";
        multiresStrs.push_back(oss.str());
    }

    for (int i=0; i<nGridPts.size()-1; i++) {
        float cf = 1.0 / (nGridPts[nGridPts.size()-1] / nGridPts[i]);
        multiresCFs.push_back(cf);
    }
    multiresCFs.push_back(1.0);


    int refLevelReq = rParams->GetRefinementLevel();

    int refLevel = refLevelReq < 0 ? 0 : refLevelReq;
    refLevel = refLevelReq >= multiresCFs.size() ?
        multiresCFs.size()-1 : refLevelReq;

    _vComboBox->SetOptions( multiresStrs );
    _vComboBox->SetIndex( refLevelReq );
}
