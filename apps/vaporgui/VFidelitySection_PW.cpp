#include <sstream>

#include "vapor/RenderParams.h"

// We must remember to explicitly include PWidget.h in all .cpp files that use 
// HLI PWidgets.  A very complex compiler error arises if we forget to do this.
// 
// Here, we are including PWidgets.h by including PGroup.h.  
//
// I'm not sure why this is the case, since we should already have PWidget.h
// in this file, through the following sequence of includes:
//
// > #include "VFidelitySection_PW.h"
//   > #include "PLODSelectorHLI.h"
//     > #include "PLODSelector.h"
//       > #include "PEnumDropdown.h"
//         > #include "PLineItem.h"
//           > #include "PWidget.h"
//

//#define MAKE_COMPILER_ERROR

#ifdef MAKE_COMPILER_ERROR
    // Do not include PWidget.h through PGroup.h, or any other means
    //
    // Also, do not use PGroup for layout management
#else
    #include "PGroup.h"
#endif



#include "VFidelitySection_PW.h"   
#include "VFidelitySection.h"     // This is where VFidelityButtons is defined

const std::string VFidelitySection_PW::_sectionTitle = "Data Fidelity";

VFidelitySection_PW::VFidelitySection_PW()
    : VSection( _sectionTitle )
{
    _fidelityButtons = new VFidelityButtons();
    layout()->addWidget( _fidelityButtons );

#ifdef MAKE_COMPILER_ERROR

#else  // Use PGroup
    _pGroup = new PGroup();
    layout()->addWidget( _pGroup );
#endif

    _plodHLI = new PLODSelectorHLI<VAPoR::RenderParams>(
        &VAPoR::RenderParams::GetCompressionLevel,
        &VAPoR::RenderParams::SetCompressionLevel
    );
#ifdef MAKE_COMPILER_ERROR
    layout()->addWidget( _plodHLI );
#else
    _pGroup->Add( _plodHLI );
#endif

    _pRefHLI = new PRefinementSelectorHLI<VAPoR::RenderParams>(
        &VAPoR::RenderParams::GetRefinementLevel,
        &VAPoR::RenderParams::SetRefinementLevel
    );
#ifdef MAKE_COMPILER_ERROR
    layout()->addWidget( _pRefHLI );
#else
    _pGroup->Add( _pRefHLI );
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

#ifdef MAKE_COMPILER_ERROR
    _pRefHLI->Update( _rParams, _paramsMgr, _dataMgr );
    _plodHLI->Update( _rParams, _paramsMgr, _dataMgr );
#else
    _pGroup->Update( _rParams, _paramsMgr, _dataMgr );
#endif

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
