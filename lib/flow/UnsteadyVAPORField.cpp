#include "vapor/UnsteadyVAPORField.h"
#include "vapor/Particle.h"
#include <cmath>

using namespace flow;

UnsteadyVAPORField::UnsteadyVAPORField()
{
    IsSteady = false;
}

// Destructor
UnsteadyVAPORField::~UnsteadyVAPORField()
{ 
    this->DestroyGrids();
}

int
UnsteadyVAPORField::AddTimeStep( const VGrid* u, const VGrid* v, const VGrid* w,
                                 const VGrid* val, float time )
{
    _velArrU.push_back( u );
    _velArrV.push_back( v );
    _velArrW.push_back( w );
    //_scalarArr.push_back( val );
    _timestamps.push_back( time );
    return 0;
}

void
UnsteadyVAPORField::DestroyGrids()
{
    for( const auto& p : _velArrU )
        delete p;
    _velArrU.clear();
    for( const auto& p : _velArrV )
        delete p;
    _velArrV.clear();
    for( const auto& p : _velArrW )
        delete p;
    _velArrW.clear();
    //for( const auto& p : _scalarArr )
    //    delete p;
    //_scalarArr.clear();
}

int
UnsteadyVAPORField::GetVelocity( float time, const glm::vec3& pos, glm::vec3& vel )const
{
    // First test if we have this time step
    size_t floor;
    int rv  = _locateTimestamp( time, floor );
    if( rv != 0 )
        return rv;

    // Second test if this position is inside of the volume
    if( !InsideVolume( time, pos ) )
        return OUT_OF_FIELD;

    // Now we retrieve the velocity of this position at time step "floor"
    const std::vector<double> coords {pos.x, pos.y, pos.z};
    float u = _velArrU[ floor ]->GetValue( coords );
    float v = _velArrV[ floor ]->GetValue( coords );
    float w = _velArrW[ floor ]->GetValue( coords );
    //   Need to do: examine u, v, w are not missing value.
    glm::vec3 velFloor( u, v, w );

    // If time is greater than _timestamps[floor], we also need to retrieve _timestamps[floor+1] 
    //   We could probably still return velFloor if the time difference is small enough
    if( time == _timestamps[floor] )
    {
        vel = velFloor;
        return 0;
    }
    else
    {
        float u1 = _velArrU[ floor+1 ]->GetValue( coords );
        float v1 = _velArrV[ floor+1 ]->GetValue( coords );
        float w1 = _velArrW[ floor+1 ]->GetValue( coords );
        //   Need to do: examine u, v, w are not missing value.
        glm::vec3 velCeil( u1, v1, w1 );
        float weight = (time - _timestamps[floor]) / (_timestamps[floor+1] - _timestamps[floor]);
        vel = glm::mix( velFloor, velCeil, weight );
        return 0;
    }
}

#if 0
int
UnsteadyVAPORField::GetScalar( float time, const glm::vec3& pos, float& val )const
{
    // First test if we have this time step
    size_t floor;
    int rv  = _locateTimestamp( time, floor );
    if( rv != 0 )
        return rv;

    // Second test if this position is inside of the volume
    if( !InsideVolume( time, pos ) )
        return OUT_OF_FIELD;

    // Now we retrieve the velocity of this position at time step "floor"
    const std::vector<double> coords {pos.x, pos.y, pos.z};
    float valFloor = _scalarArr[ floor ]->GetValue( coords );
    //   Need to do: examine valFloor is not missing value.

    // If time is greater than _timestamps[floor], we also need to retrieve _timestamps[floor+1] 
    //   We could probably still return velFloor if the time difference is small enough
    if( time == _timestamps[floor] )
    {
        val = valFloor;
        return 0;
    }
    else
    {
        float valCeil = _scalarArr[ floor+1 ]->GetValue( coords );
        //   Need to do: examine valCeil is not missing value.
        float weight = (time - _timestamps[floor]) / (_timestamps[floor+1] - _timestamps[floor]);
        val = glm::mix( valFloor, valCeil, weight );
        return 0;
    }
}
#endif

bool
UnsteadyVAPORField::InsideVolume( float time, const glm::vec3& pos ) const
{
    // First test if we have this time step
    size_t floor;
    int rv  = _locateTimestamp( time, floor );
    if( rv != 0 )
        return false;

    // Second test if pos is inside of time step "floor"
    const std::vector<double> coords { pos.x, pos.y, pos.z }; 
    if( !_velArrU[ floor ]->InsideGrid( coords ) )
        return false;
    if( !_velArrV[ floor ]->InsideGrid( coords ) )
        return false;
    if( !_velArrW[ floor ]->InsideGrid( coords ) )
        return false;
    //if( (!_scalarArr.empty()) && (!_scalarArr[ floor ]->InsideGrid( coords )) )
    //    return false;

    // If time is larger than _timestamps[floor], we also need to test _timestamps[floor+1]
    if( time > _timestamps[floor] )
    {
        if( !_velArrU[ floor+1 ]->InsideGrid( coords ) )
            return false;
        if( !_velArrV[ floor+1 ]->InsideGrid( coords ) )
            return false;
        if( !_velArrW[ floor+1 ]->InsideGrid( coords ) )
            return false;
        //if( (!_scalarArr.empty()) && (!_scalarArr[ floor+1 ]->InsideGrid( coords )) )
        //    return false;
    }
    
    return true;
}

int
UnsteadyVAPORField::_locateTimestamp( float time, size_t& floor ) const
{
    if( _timestamps.size() == 0 )
        return NOT_CONTAIN_TIME;
    if( _timestamps.size() == 1 )
    {
        if( _timestamps[0] != time )
            return NOT_CONTAIN_TIME;
        else
        {
            floor = 0;
            return 0;
        }
    }

    if( time < _timestamps.front() || time > _timestamps.back() )
        return NOT_CONTAIN_TIME;
    else
    {
        floor = _binarySearch( _timestamps, time, 0, _timestamps.size() - 1 );
        return 0;
    }
}

template<typename T> size_t
UnsteadyVAPORField::_binarySearch( const std::vector<T>& vec, T val, 
                                   size_t begin, size_t end        ) const
{
    if( begin + 1 == end )
    {
        if( val == vec[end] )
            return end;
        else
            return begin;
    }

    size_t middle = begin + (end - begin) / 2;
    if( val == vec[middle] )
        return middle;
    else if( val < vec[middle] )
        return _binarySearch( vec, val, begin, middle );
    else
        return _binarySearch( vec, val, middle, end );
}

int  
UnsteadyVAPORField::GetExtents( float time, glm::vec3& minExt, glm::vec3& maxExt ) const
{
    size_t floor;
    int rv = _locateTimestamp( time, floor );
    if( rv != 0 )
        return NOT_CONTAIN_TIME;

    // find the cloest time step
    size_t idx = floor;
    if( floor < _timestamps.size() - 1 )
    {
        size_t ceil = floor + 1;
        if( _timestamps[ceil] - time > time - _timestamps[floor] )
            idx = ceil;
    }

    std::vector<double>            gridMin, gridMax;
    _velArrU[idx]->GetUserExtents( gridMin, gridMax );
    glm::vec3 uMin( gridMin.at(0), gridMin.at(1), gridMin.at(2) );
    glm::vec3 uMax( gridMax.at(0), gridMax.at(1), gridMax.at(2) );

    _velArrV[idx]->GetUserExtents( gridMin, gridMax );
    glm::vec3 vMin( gridMin.at(0), gridMin.at(1), gridMin.at(2) );
    glm::vec3 vMax( gridMax.at(0), gridMax.at(1), gridMax.at(2) );

    _velArrW[idx]->GetUserExtents( gridMin, gridMax );
    glm::vec3 wMin( gridMin.at(0), gridMin.at(1), gridMin.at(2) );
    glm::vec3 wMax( gridMax.at(0), gridMax.at(1), gridMax.at(2) );

    minExt = glm::min( uMin, glm::min( vMin, wMin ) );
    maxExt = glm::max( uMax, glm::max( vMax, wMax ) );

    /*if( _scalarArr.size() > 0 )
    {
        _scalarArr[idx]->GetUserExtents( gridMin, gridMax );
        glm::vec3 sMin( gridMin.at(0),   gridMin.at(1), gridMin.at(2) );
        glm::vec3 sMax( gridMax.at(0),   gridMax.at(1), gridMax.at(2) );

        minExt = glm::min( minExt, sMin );
        maxExt = glm::max( maxExt, sMax );
    }*/
    
    return 0;
}
