#include <sstream>

#include "vapor/RenderParams.h"

// I don't understand why we need to include PWidget.h to use HLI widgets.
// Commend the line below, and many errors arise that are not clear to me.
// -Scott
#include "PWidget.h"

#include "VPFidelitySection.h"
#include "VFidelitySection.h"

const std::string VPFidelitySection::_sectionTitle = "Data Fidelity (VPFidelitySection)";

VPFidelitySection::VPFidelitySection()
    : VSection( _sectionTitle )
{
    _fidelityButtons = new VFidelityButtons();
    layout()->addWidget( _fidelityButtons );

    _plodHLI = new PLODSelectorHLI<VAPoR::RenderParams>(
        &VAPoR::RenderParams::GetCompressionLevel,
        &VAPoR::RenderParams::SetCompressionLevel
    );
    layout()->addWidget( _plodHLI );

    _pRefHLI = new PRefinementSelectorHLI<VAPoR::RenderParams>(
        &VAPoR::RenderParams::GetRefinementLevel,
        &VAPoR::RenderParams::SetRefinementLevel
    );
    layout()->addWidget( _pRefHLI );
}


void VPFidelitySection::Reinit( VariableFlags varFlags ) {
    _variableFlags = varFlags;
   
    _fidelityButtons->Reinit( _variableFlags);
    _plodHLI->Reinit( _variableFlags );
    _pRefHLI->Reinit( _variableFlags );
}

void VPFidelitySection::Update(
    VAPoR::RenderParams* rParams,
    VAPoR::ParamsMgr*    paramsMgr,
    VAPoR::DataMgr*      dataMgr
) {
    VAssert(dataMgr);
    VAssert(paramsMgr);
    VAssert(rParams);

    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _rParams = rParams;

    _plodHLI->Update( rParams, _paramsMgr, _dataMgr );
    _pRefHLI->Update( rParams, _paramsMgr, _dataMgr );
    _fidelityButtons->Update( _rParams, _paramsMgr, _dataMgr );
}

void VPFidelitySection::setNumRefinements( int num ) {
    _rParams->SetRefinementLevel(num);
}

void VPFidelitySection::setCompRatio( int num ) {
    _rParams->SetCompressionLevel(num);
}

std::string VPFidelitySection::GetCurrentLodString() const {
    return _currentLodStr;
}

std::string VPFidelitySection::GetCurrentMultiresString() const {
    return _currentMultiresStr;
}
