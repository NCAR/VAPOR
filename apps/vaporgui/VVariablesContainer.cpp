#include "VVariablesContainer.h"
#include "VVariablesSection.h"
#include "VFidelitySection.h"

#include "vapor/RenderParams.h"

#include <QHBoxLayout>

VVariablesContainer::VVariablesContainer() 
    : VContainer( new QVBoxLayout )
{
    _variablesSection = new VVariablesSection();
    layout()->addWidget( _variablesSection );
    
    _fidelitySection  = new VFidelitySection();    
    layout()->addWidget( _fidelitySection );

    layout()->addItem(
        new QSpacerItem( 1, 2000, QSizePolicy::Minimum, QSizePolicy::Maximum )
    );
}

void VVariablesContainer::Reinit( VariableFlags varFlags,
                                  DimFlags      dimFlags
 ) {
    _variablesSection->Reinit( varFlags, dimFlags );
    _fidelitySection->Reinit(  varFlags );
}

void VVariablesContainer::Update(
    VAPoR::RenderParams* rParams,
    VAPoR::ParamsMgr*    paramsMgr,
    VAPoR::DataMgr*      dataMgr
) {
    _variablesSection->Update( rParams, paramsMgr, dataMgr );
    _fidelitySection->Update(  rParams, paramsMgr, dataMgr );
}
