#include "vapor/SteadyVAPORVelocity.h"
#include "vapor/Particle.h"

using namespace flow;

// Constructor
SteadyVAPORVelocity::SteadyVAPORVelocity()
{
    IsSteady    = true;

    _gridU  = nullptr;
    _gridV  = nullptr;
    _gridW  = nullptr;
}

// Destructor
SteadyVAPORVelocity::~SteadyVAPORVelocity()
{
    this->DestroyGrids();
}

void
SteadyVAPORVelocity::DestroyGrids()
{
    if( _gridU )    delete _gridU;
    if( _gridV )    delete _gridV;
    if( _gridW )    delete _gridW;
    _gridU  = nullptr;
    _gridV  = nullptr;
    _gridW  = nullptr;
}


int
SteadyVAPORVelocity::GetVelocity( float t, const glm::vec3& pos, glm::vec3& vel ) const
{
    if( !_gridU || !_gridV || !_gridW )
        return NO_VECTOR_FIELD_YET ;

    if( !InsideVolume( t, pos ) )
        return OUT_OF_FIELD; 

    const std::vector<double> coords {pos.x, pos.y, pos.z};
    float u = _gridU->GetValue( coords );
    float v = _gridV->GetValue( coords );
    float w = _gridW->GetValue( coords );
    // Need to do: examine u, v, w are not missing value.
    vel = glm::vec3( u, v, w );

    return 0;
}

bool
SteadyVAPORVelocity::InsideVolume( float time, const glm::vec3& pos ) const
{
    std::vector<double> coords { pos.x, pos.y, pos.z }; 
    if( !_gridU->InsideGrid( coords ) )
        return false;
    if( !_gridV->InsideGrid( coords ) )
        return false;
    if( !_gridW->InsideGrid( coords ) )
        return false;

    return true;
}

void
SteadyVAPORVelocity::UseGrids( const VGrid* u, const VGrid* v, const VGrid* w )
{
    _gridU = u;
    _gridV = v;
    _gridW = w;
}

int  
SteadyVAPORVelocity::GetExtents( float time, glm::vec3& minExt, glm::vec3& maxExt ) const
{
    if( !_gridU || !_gridV || !_gridW )
        return NO_VECTOR_FIELD_YET ;
    
    std::vector<double>         gridMin, gridMax;
    _gridU->GetUserExtents( gridMin, gridMax );
    glm::vec3 uMin( gridMin.at(0), gridMin.at(1), gridMin.at(2) );
    glm::vec3 uMax( gridMax.at(0), gridMax.at(1), gridMax.at(2) );

    _gridV->GetUserExtents( gridMin, gridMax );
    glm::vec3 vMin( gridMin.at(0), gridMin.at(1), gridMin.at(2) );
    glm::vec3 vMax( gridMax.at(0), gridMax.at(1), gridMax.at(2) );

    _gridW->GetUserExtents( gridMin, gridMax );
    glm::vec3 wMin( gridMin.at(0), gridMin.at(1), gridMin.at(2) );
    glm::vec3 wMax( gridMax.at(0), gridMax.at(1), gridMax.at(2) );

    minExt = glm::min( uMin, glm::min( vMin, wMin ) );
    maxExt = glm::max( uMax, glm::max( vMax, wMax ) );

    return 0;
}
