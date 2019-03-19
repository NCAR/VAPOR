#include "vapor/FlowParams.h"

using namespace VAPoR;

const std::string FlowParams::_isSteadyTag = "isSteadyTag";
//const std::string FlowParams::_alreadyAdvectionStepTag = "alreadyAdvectionStepTag";

static RenParamsRegistrar<FlowParams> registrar(FlowParams::GetClassType());

// Constructor
FlowParams::FlowParams(   DataMgr*                 dataManager,
                          ParamsBase::StateSave*   stateSave )
          : RenderParams( dataManager,
                          stateSave, 
                          FlowParams::GetClassType(),
                          3 /* max dim */ )
{
    SetDiagMsg("FlowParams::FlowParams() this=%p", this);
}

FlowParams::FlowParams(   DataMgr*                dataManager, 
                          ParamsBase::StateSave*  stateSave,
                          XmlNode*                node )
       : RenderParams(    dataManager, 
                          stateSave, 
                          node,
                          3 /* max dim */ )
{
    SetDiagMsg("FlowParams::FlowParams() this=%p", this);
}

// Destructor
FlowParams::~FlowParams()
{
    SetDiagMsg( "FlowParams::~FlowParams() this=%p", this );
}

void
FlowParams::SetIsSteady( bool steady)
{
	SetValueLong( _isSteadyTag, "are we using steady advection", long(steady) );
}

bool
FlowParams::GetIsSteady() const
{
	long rv = GetValueLong( _isSteadyTag, long(false) );
    return bool(rv);
}

/*
void
FlowParams::SetAlreadyAdvectionStep( long i )
{
    SetValueLong( _alreadyAdvectionStepTag, "how many step already", i );
}
long
FlowParams::GetAlreadyAdvectionStep( ) const
{
    return GetValueLong( _alreadyAdvectionStepTag, 0 );
}
*/
