#include "vapor/SteadyVAPORField.h"
#include "vapor/Particle.h"

using namespace flow;

// Constructor
SteadyVAPORField::SteadyVAPORField()
{
    IsSteady    = true;

    _velocityU  = nullptr;
    _velocityV  = nullptr;
    _velocityW  = nullptr;
    _scalar     = nullptr;
}

// Destructor
SteadyVAPORField::~SteadyVAPORField()
{
    this->DestroyGrids();
}

void
SteadyVAPORField::DestroyGrids()
{
    if( _velocityU )    delete _velocityU;
    if( _velocityV )    delete _velocityV;
    if( _velocityW )    delete _velocityW;
    if( _scalar    )    delete _scalar;
    _velocityU  = nullptr;
    _velocityV  = nullptr;
    _velocityW  = nullptr;
    _scalar     = nullptr;
}


int
SteadyVAPORField::GetVelocity( float t, const glm::vec3& pos, glm::vec3& vel ) const
{
    if( !_velocityU || !_velocityV || !_velocityW )
        return NO_VECTOR_FIELD_YET ;

    if( !InsideVolume( t, pos ) )
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
SteadyVAPORField::GetScalar( float t, const glm::vec3& pos, float& val ) const
{
    if( !_scalar )
        return NO_VALUE_FIELD_YET ;

    std::vector<double> coords {pos.x, pos.y, pos.z};
    float v = _scalar->GetValue( coords );
    // Need to do: examine v is not missing value.
    val = v;

    return 0;
}

bool
SteadyVAPORField::InsideVolume( float time, const glm::vec3& pos ) const
{
    std::vector<double> coords { pos.x, pos.y, pos.z }; 
    if( !_velocityU->InsideGrid( coords ) )
        return false;
    if( !_velocityV->InsideGrid( coords ) )
        return false;
    if( !_velocityW->InsideGrid( coords ) )
        return false;

    // If there's field value, we test it too
    if( (_scalar != nullptr) && (!_scalar->InsideGrid( coords )) )
        return false;

    return true;
}

void
SteadyVAPORField::UseVelocities( const VGrid* u, const VGrid* v, const VGrid* w )
{
    _velocityU = u;
    _velocityV = v;
    _velocityW = w;
}

void 
SteadyVAPORField::UseScalar( const VGrid* val )
{
    _scalar        = val;
    HasScalarValue = true;
}

int  
SteadyVAPORField::GetExtents( float time, glm::vec3& minExt, glm::vec3& maxExt ) const
{
    if( !_velocityU || !_velocityV || !_velocityW )
        return NO_VECTOR_FIELD_YET ;
    
    std::vector<double>         gridMin, gridMax;
    _velocityU->GetUserExtents( gridMin, gridMax );
    glm::vec3 uMin( gridMin.at(0), gridMin.at(1), gridMin.at(2) );
    glm::vec3 uMax( gridMax.at(0), gridMax.at(1), gridMax.at(2) );

    _velocityV->GetUserExtents( gridMin, gridMax );
    glm::vec3 vMin( gridMin.at(0), gridMin.at(1), gridMin.at(2) );
    glm::vec3 vMax( gridMax.at(0), gridMax.at(1), gridMax.at(2) );

    _velocityW->GetUserExtents( gridMin, gridMax );
    glm::vec3 wMin( gridMin.at(0), gridMin.at(1), gridMin.at(2) );
    glm::vec3 wMax( gridMax.at(0), gridMax.at(1), gridMax.at(2) );

    minExt = glm::min( uMin, glm::min( vMin, wMin ) );
    maxExt = glm::max( uMax, glm::max( vMax, wMax ) );

    if( _scalar )
    {
        _scalar->GetUserExtents( gridMin, gridMax );
        glm::vec3 sMin( gridMin.at(0), gridMin.at(1), gridMin.at(2) );
        glm::vec3 sMax( gridMax.at(0), gridMax.at(1), gridMax.at(2) );

        minExt = glm::min( minExt, sMin );
        maxExt = glm::max( maxExt, sMax );
    }

    return 0;
}
