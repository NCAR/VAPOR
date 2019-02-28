#include "UnsteadyVAPORField.h"
#include <cmath>

using namespace flow;

UnsteadyVAPORField::UnsteadyVAPORField()
{
    IsSteady = false;
}

UnsteadyVAPORField::~UnsteadyVAPORField()
{ }

int
UnsteadyVAPORField::AddTimeStep( const VGrid* u, const VGrid* v, const VGrid* w,
                                 const VGrid* val, float time )
{
    _velArrU.push_back( u );
    _velArrV.push_back( v );
    _velArrW.push_back( w );
    _valueArr.push_back( val );
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
    for( const auto& p : _valueArr )
        delete p;
    _valueArr.clear();
}

bool
UnsteadyVAPORField::InsideVelocityField( float time, const glm::vec3& pos ) const
{
    float floor = std::floor( time );
    float ceil  = std::ceil(  time );

    return true;
    // First test time step "floor"
    //const VGrid* u = _velArr
}

int
UnsteadyVAPORField::_locateTimestamps( float time, size_t& floor ) const
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
