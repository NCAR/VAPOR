#include "SteadyVAPORField.h"

using namespace flow;

SteadyVAPORField::SteadyVAPORField()
{
    IsSteady   = true;

    _velocityU = nullptr;
    _velocityV = nullptr;
    _velocityW = nullptr;
    _value     = nullptr;
}

SteadyVAPORField::~SteadyVAPORField()
{
    _velocityU = nullptr;
    _velocityV = nullptr;
    _velocityW = nullptr;
    _value     = nullptr;
}

bool
SteadyVAPORField::_isReady() const
{
    return( _velocityU && _velocityV && _velocityW );
}

void
SteadyVAPORField::DestroyGrids()
{
    if( _velocityU )    delete _velocityU;
    if( _velocityV )    delete _velocityV;
    if( _velocityW )    delete _velocityW;
    if( _value )        delete _value;
}


int
SteadyVAPORField::Get( float t, const glm::vec3& pos, glm::vec3& vel ) const
{
    if( !_isReady() )
        return NO_VECTOR_FIELD_YET ;

    if( !InsideField( t, pos ) )
        return OUT_OF_FIELD; 

    std::vector<double> coords {pos.x, pos.y, pos.z};
    float u = _velocityU->GetValue( coords );
    float v = _velocityV->GetValue( coords );
    float w = _velocityW->GetValue( coords );
    // Need to do: examine u, v, w are not missing value.
    vel = glm::vec3( u, v, w );

    return 0;
}

bool
SteadyVAPORField::InsideField( float time, const glm::vec3& pos ) const
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

    // Collect user extents of these grids
    std::vector<double> min, max;
    _velocityU->GetUserExtents( min, max );
    glm::vec3 min1( (min[0]), (min[1]), (min[2]) );
    glm::vec3 max1( (max[0]), (max[1]), (max[2]) );

    _velocityV->GetUserExtents( min, max );
    glm::vec3 min2( (min[0]), (min[1]), (min[2]) );
    glm::vec3 max2( (max[0]), (max[1]), (max[2]) );

    _velocityW->GetUserExtents( min, max );
    glm::vec3 min3( (min[0]), (min[1]), (min[2]) );
    glm::vec3 max3( (max[0]), (max[1]), (max[2]) );

    _fieldMin = glm::min( min1, glm::min(min2, min3) );
    _fieldMax = glm::max( max1, glm::max(max2, max3) );
}

void 
SteadyVAPORField::UseValueField( const VGrid* val )
{
    _value = val;

    // Collect user extents for this grid
    std::vector<double> min, max;
    _value->GetUserExtents( min, max );
    glm::vec3 min1( (min[0]), (min[1]), (min[2]) );
    glm::vec3 max1( (max[0]), (max[1]), (max[2]) );

    _fieldMin = glm::min( _fieldMin, min1 );
    _fieldMax = glm::max( _fieldMax, max1 );
}
