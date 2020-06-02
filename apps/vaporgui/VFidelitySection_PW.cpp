#include <sstream>

#include "vapor/RenderParams.h"

#define COMPILER_ERROR false


#ifndef COMPILER_ERROR
#include "PGroup.h"
#endif

// If we don't use a PGroup here, we need to include PWidget.h.  However,
// this code should not need to.  We never directly reference a PWidget.
// 
// If we don't include PWidget.h in the above circumstance, the compiler
// throws a complex error within the PWidgetHLIBase macro.
//
// -Scott
//#include "PWidget.h"


#include "VFidelitySection_PW.h"
#include "VFidelitySection.h"   // This is where VFidelityButtons is defined

const std::string VFidelitySection_PW::_sectionTitle = "Data Fidelity";

VFidelitySection_PW::VFidelitySection_PW()
    : VSection( _sectionTitle )
{
    _fidelityButtons = new VFidelityButtons();
    layout()->addWidget( _fidelityButtons );

    _pGroup = new PGroup();
    layout()->addWidget( _pGroup );

    _plodHLI = new PLODSelectorHLI<VAPoR::RenderParams>(
        &VAPoR::RenderParams::GetCompressionLevel,
        &VAPoR::RenderParams::SetCompressionLevel
    );
#ifndef COMPILER_ERROR
    _pGroup->Add( _plodHLI );
#else
    layout()->addWidget( _plodHLI );
#endif

    _pRefHLI = new PRefinementSelectorHLI<VAPoR::RenderParams>(
        &VAPoR::RenderParams::GetRefinementLevel,
        &VAPoR::RenderParams::SetRefinementLevel
    );
#ifndef COMPILER_ERROR
    _pGroup->Add( _pRefHLI );
#else
    layout()->addWidget( _pRefHLI );
#endif
}


void VFidelitySection_PW::Reinit( VariableFlags varFlags ) {
    _variableFlags = varFlags;
   
    _fidelityButtons->Reinit( _variableFlags);
    _plodHLI->Reinit( _variableFlags );
    _pRefHLI->Reinit( _variableFlags );
}

void VFidelitySection_PW::Update(
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

    _pGroup->Update( _rParams, _paramsMgr, _dataMgr );

    _fidelityButtons->Update( _rParams, _paramsMgr, _dataMgr );
}

void VFidelitySection_PW::setNumRefinements( int num ) {
    _rParams->SetRefinementLevel(num);
}

void VFidelitySection_PW::setCompRatio( int num ) {
    _rParams->SetCompressionLevel(num);
}

std::string VFidelitySection_PW::GetCurrentLodString() const {
    return _currentLodStr;
}

std::string VFidelitySection_PW::GetCurrentMultiresString() const {
    return _currentMultiresStr;
}
