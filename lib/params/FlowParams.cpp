#include "vapor/FlowParams.h"

using namespace VAPoR;

const std::string FlowParams::_velocityUTag = "velocityUTag";
const std::string FlowParams::_velocityVTag = "velocityVTag";
const std::string FlowParams::_velocityWTag = "velocityWTag";

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
FlowParams::SetVelocityVarNameU( std::string& s )
{
	SetValueString( _velocityUTag, "Specify U velocity name", s );
}

void
FlowParams::SetVelocityVarNameV( std::string& s )
{
	SetValueString( _velocityVTag, "Specify V velocity name", s );
}

void
FlowParams::SetVelocityVarNameW( std::string& s )
{
	SetValueString( _velocityWTag, "Specify W velocity name", s );
}

std::string
FlowParams::GetVelocityVarNameU() const
{
	return GetValueString( _velocityUTag, "" );
}

std::string
FlowParams::GetVelocityVarNameV() const
{
	return GetValueString( _velocityVTag, "" );
}

std::string
FlowParams::GetVelocityVarNameW() const
{
	return GetValueString( _velocityWTag, "" );
}
