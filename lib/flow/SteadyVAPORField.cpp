#include "vapor/SteadyVAPORField.h"

using namespace flow;

SteadyVAPORField::SteadyVAPORField()
{
    IsSteady   = true;

    _velocityU  = nullptr;
    _velocityV  = nullptr;
    _velocityW  = nullptr;
    _fieldValue = nullptr;
}

SteadyVAPORField::~SteadyVAPORField()
{
    _velocityU  = nullptr;
    _velocityV  = nullptr;
    _velocityW  = nullptr;
    _fieldValue = nullptr;
}

void
SteadyVAPORField::DestroyGrids()
{
    if( _velocityU )    delete _velocityU;
    if( _velocityV )    delete _velocityV;
    if( _velocityW )    delete _velocityW;
    if( _fieldValue )   delete _fieldValue;
}


int
SteadyVAPORField::GetVelocity( float t, const glm::vec3& pos, glm::vec3& vel ) const
{
    if( !_velocityU || !_velocityV || !_velocityW )
        return NO_VECTOR_FIELD_YET ;

    if( !InsideVelocityField( t, pos ) )
        return OUT_OF_FIELD; 

    const std::vector<double> coords {pos.x, pos.y, pos.z};
    float u = _velocityU->GetValue( coords );
    float v = _velocityV->GetValue( coords );
    float w = _velocityW->GetValue( coords );
    // Need to do: examine u, v, w are not missing value.
    vel = glm::vec3( u, v, w );

    return 0;
}

int
SteadyVAPORField::GetFieldValue( float t, const glm::vec3& pos, float& val ) const
{
    if( !_fieldValue )
        return NO_VALUE_FIELD_YET ;

    std::vector<double> coords {pos.x, pos.y, pos.z};
    float v = _fieldValue->GetValue( coords );
    // Need to do: examine v is not missing value.
    val = v;

    return 0;
}

bool
SteadyVAPORField::InsideVelocityField( float time, const glm::vec3& pos ) const
{
    std::vector<double> coords { pos.x, pos.y, pos.z }; 
    if( !_velocityU->InsideGrid( coords ) )
        return false;
    if( !_velocityV->InsideGrid( coords ) )
        return false;
    if( !_velocityW->InsideGrid( coords ) )
        return false;

    return true;
}

void
SteadyVAPORField::UseVelocityField( const VGrid* u, const VGrid* v, const VGrid* w )
{
    _velocityU = u;
    _velocityV = v;
    _velocityW = w;
}

void 
SteadyVAPORField::UseFieldValue( const VGrid* val )
{
    _fieldValue = val;
}
