#include <sstream>
#include <assert.h>

#include <VComboBox.h>
#include <vapor/RenderParams.h>

#include "PLODSelector.h"
#include "VariableGetter.h"

PLODSelector::PLODSelector()
: PEnumDropdown("", {}, {}, "Level of Detail")
{}

void PLODSelector::Reinit( VariableFlags varFlags ) {
    _variableFlags = varFlags;
}   

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
    VAPoR::RenderParams *rParams = dynamic_cast<VAPoR::RenderParams*>(getParams());
    assert(rParams && "Params must be RenderParams");
    static_cast<void>(rParams);  // Silence unused variable warning

    VAPoR::DataMgr* dataMgr = getDataMgr();

    std::string varName = getCurrentVariable( 
        rParams, 
        dataMgr,
        _variableFlags
    );

    std::vector <long> lodCFs;
    std::vector <std::string> lodStrs;

    vector <size_t> cratios = dataMgr->GetCRatios(varName);

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
