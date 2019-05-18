#include "vapor/VaporField.h"

using namespace flow;

// Constructor
VaporField::VaporField() : _recentGridLimit( 12 )
{
    _datamgr = nullptr;
    _params  = nullptr;
}

VaporField::VaporField(int limit) : _recentGridLimit( limit )
{
    _datamgr = nullptr;
    _params  = nullptr;
}

// Destructor
VaporField::~VaporField()
{ }


bool
VaporField::InsideVolumeVelocity( float time, const glm::vec3& pos )
{
    const std::vector<double> coords{ pos.x, pos.y, pos.z };
    const VAPoR::Grid* grid = nullptr;
    assert( _isReady() );

    // In case of steady field, we only check a specific time step
    if( IsSteady )
    {
        size_t currentTS = _params->GetCurrentTimestep();
        for( auto v : VelocityNames )   // cannot use reference here...
            if( !v.empty() )
            {
                int rv = _getAGrid( currentTS, v, &grid );
                assert( rv == 0 );
                if( !grid->InsideGrid( coords ) )
                    return false;
            }
    }
    else    // we check two time steps
    {
        // First check if the query time is within range
        if( time < _timestamps.front() || time > _timestamps.back() )
            return false;

        // Then locate the 2 time steps
        size_t floor;
        int rv  = LocateTimestamp( time, floor );
        if( rv != 0 ) return false;

        // Second test if pos is inside of time step "floor"
        for( auto v : VelocityNames )   // cannot use references...
            if( !v.empty() )
            {
                rv = _getAGrid( floor, v, &grid );
                assert( rv == 0 );
                if( !grid->InsideGrid( coords ) )
                    return false;
            }

        // If time is larger than _timestamps[floor], we also need to test _timestamps[floor+1]
        if( time > _timestamps[floor] )
        {
            for( auto v : VelocityNames )   // cannot use references
                if( !v.empty() )
                {
                    rv = _getAGrid( floor + 1, v, &grid );
                    assert( rv == 0 );
                    if( !grid->InsideGrid( coords ) )
                        return false;
                }
        }
    }

    return true;
}


bool
VaporField::InsideVolumeScalar( float time, const glm::vec3& pos )
{
    if( ScalarName.empty() )
        return NO_FIELD_YET;
    std::string scalarname = ScalarName;    // const requirement...
    const std::vector<double> coords{ pos.x, pos.y, pos.z };
    const VAPoR::Grid* grid = nullptr;
    assert( _isReady() );

    // In case of steady field, we only check a specific time step
    if( IsSteady )
    {
        size_t currentTS = _params->GetCurrentTimestep();
        int rv = _getAGrid( currentTS, scalarname, &grid );
        assert( rv == 0 );
        if( !grid->InsideGrid( coords ) )
            return false;
    }
    else    // we check two time steps
    {
        // First check if the query time is within range
        if( time < _timestamps.front() || time > _timestamps.back() )
            return false;

        // Then locate the 2 time steps
        size_t floor;
        int rv  = LocateTimestamp( time, floor );
        if( rv != 0 ) return false;

        // Second test if pos is inside of time step "floor"
        rv = _getAGrid( floor, scalarname, &grid );
        assert( rv == 0 );
        if( !grid->InsideGrid( coords ) )
            return false;

        // If time is larger than _timestamps[floor], we also need to test _timestamps[floor+1]
        if( time > _timestamps[floor] )
        {
            rv = _getAGrid( floor + 1, scalarname, &grid );
            assert( rv == 0 );
            if( !grid->InsideGrid( coords ) )
                return false;
        }
    }

    return true;
}

void
VaporField::GetFirstStepVelocityIntersection( glm::vec3& minxyz, glm::vec3& maxxyz )
{
    const VAPoR::Grid* grid = nullptr;
    std::vector<double> min[3], max[3];

    for( int i = 0; i < 3; i++ )
    {
        auto varname = VelocityNames[i];
        int rv       = _getAGrid( 0, varname, &grid );
        assert(  rv == 0 );
        grid->GetUserExtents( min[i], max[i] );
    }

    minxyz = glm::vec3 ( min[0][0], min[0][1], min[0][2] );
    maxxyz = glm::vec3 ( max[0][0], max[0][1], max[0][2] );

    for( int i = 1; i < 3; i++ )
    {
        glm::vec3 xyz(      min[i][0], min[i][1], min[i][2] );
        minxyz = glm::max(  minxyz, xyz );
        xyz    = glm::vec3( max[i][0], max[i][1], max[i][2] );
        maxxyz = glm::min(  maxxyz, xyz );
    }
}

int
VaporField::GetVelocity( float time, const glm::vec3& pos, glm::vec3& velocity,
                         bool  checkInsideVolume )
{
    const std::vector<double> coords{ pos.x, pos.y, pos.z };
    const VAPoR::Grid* grid = nullptr;

    // First make sure the query positions are inside of the volume
    if( checkInsideVolume )
        if( !InsideVolumeVelocity( time, pos ) )
            return OUT_OF_FIELD; 

    // Retrieve the missing value velocity multiplier 
    const float mult = _params->GetVelocityMultiplier();
    glm::vec3 missingV;

    if( IsSteady )
    {
        size_t currentTS = _params->GetCurrentTimestep();
        for( int i = 0; i < 3; i++ )
        {
            auto varname = VelocityNames[i];
            int     rv  = _getAGrid( currentTS, varname, &grid );
            assert( rv == 0 );
            velocity[i] = grid->GetValue( coords );
            missingV[i] = grid->GetMissingValue();
        }
        auto hasMissing = glm::equal( velocity, missingV );
        if( glm::any( hasMissing ) )
            velocity = glm::vec3( 0.0f );
        else
            velocity *= mult;
    }
    else
    {
        // First check if the query time is within range
        if( time < _timestamps.front() || time > _timestamps.back() )
            return TIME_ERROR;

        // Then we locate the floor time step
        size_t floorTS;
        int rv  = LocateTimestamp( time, floorTS );
        assert( rv == 0 );

        // Find the velocity values at floor time step
        glm::vec3 floorVelocity, ceilVelocity;
        for( int i = 0; i < 3; i++ )
        {
            auto varname = VelocityNames[i];
            int     rv   = _getAGrid( floorTS, varname, &grid );
            assert( rv  == 0 );
            floorVelocity[i] = grid->GetValue( coords );
            missingV[i]      = grid->GetMissingValue();
        }
        auto hasMissing = glm::equal( floorVelocity, missingV );
        if( glm::any( hasMissing ) )
        {
            velocity = glm::vec3( 0.0f );
            return 0;
        }

        // Find the velocity values at the ceiling time step
        if( time == _timestamps[floorTS] )
            velocity = floorVelocity * mult; 
        else
        {
            for( int i = 0; i < 3; i++ )
            {
                auto varname = VelocityNames[i];
                int     rv   = _getAGrid( floorTS + 1, varname, &grid );
                assert( rv  == 0 );
                ceilVelocity[i] = grid->GetValue( coords );
                missingV[i]     = grid->GetMissingValue();
            }
            hasMissing = glm::equal( ceilVelocity, missingV );
            if( glm::any( hasMissing ) )
            {
                velocity = glm::vec3( 0.0f );
                return 0;
            }
            
            float weight = (time - _timestamps[floorTS]) / 
                           (_timestamps[floorTS+1] - _timestamps[floorTS]);
            velocity = glm::mix( floorVelocity, ceilVelocity, weight ) * mult;
        }
    }

    return 0;
}


int
VaporField::GetScalar( float time, const glm::vec3& pos, float& scalar,
                       bool  checkInsideVolume )
{
    if( ScalarName.empty() )
        return NO_FIELD_YET;
    if( checkInsideVolume )
        if( !InsideVolumeScalar( time, pos ) )
            return OUT_OF_FIELD;

    std::string scalarname = ScalarName;    // const requirement...

    const std::vector<double> coords{ pos.x, pos.y, pos.z };
    const VAPoR::Grid* grid = nullptr;

    if( IsSteady )
    {
        size_t currentTS = _params->GetCurrentTimestep();
        int     rv = _getAGrid( currentTS, scalarname, &grid );
        assert( rv == 0 );
        float gridV = grid->GetValue( coords );
        if( gridV  == grid->GetMissingValue() )
            scalar  = 0.0f;
        else
            scalar  = gridV;
    }
    else
    {
        // First check if the query time is within range
        if( time < _timestamps.front() || time > _timestamps.back() )
            return TIME_ERROR;

        // Then we locate the floor time step
        size_t floorTS;
        int rv  = LocateTimestamp( time, floorTS );
        assert( rv == 0 );
        rv          = _getAGrid( floorTS, scalarname, &grid );
        assert( rv == 0 );
        float floorScalar = grid->GetValue( coords );
        if( floorScalar  == grid->GetMissingValue() )
        {
            scalar = 0.0f;
            return 0;
        }

        if( time == _timestamps[floorTS] )
            scalar = floorScalar;
        else
        {
            rv = _getAGrid( floorTS + 1, scalarname, &grid );
            assert( rv == 0 );
            float ceilScalar = grid->GetValue( coords );
            if( ceilScalar  == grid->GetMissingValue() )
            {
                scalar = 0.0f;
                return 0;
            }
            float weight = (time - _timestamps[floorTS]) /
                           (_timestamps[floorTS+1] - _timestamps[floorTS]);
            scalar = glm::mix( floorScalar, ceilScalar, weight );
        }
    }

    return 0;
}


bool
VaporField::_isReady() const
{
    if( !_datamgr )         return false;
    if( !_params )          return false;

    return true;
}


void
VaporField::AssignDataManager( VAPoR::DataMgr* dmgr )
{
    _datamgr = dmgr;

    // Make a copy of the timestamps from the new data manager
    const auto& timeCoords = dmgr->GetTimeCoordinates();
    _timestamps.resize( timeCoords.size() );
    for( size_t i = 0; i < timeCoords.size(); i++ )
        _timestamps[i] = timeCoords[i];
}


void
VaporField::UpdateParams( const VAPoR::FlowParams* p )
{
    _params = p;
    // Update properties of this Field
    IsSteady = p->GetIsSteady();
    ScalarName = p->GetColorMapVariableName();
    auto velNames = p->GetFieldVariableNames();
    for( int i = 0; i < 3; i++ )
        VelocityNames[i] = velNames.at(i);
}


int
VaporField::LocateTimestamp( float time, size_t& floor ) const
{
    if( _timestamps.size() == 0 )
        return TIME_ERROR;
    if( _timestamps.size() == 1 )
    {
        if( _timestamps[0] != time )
            return TIME_ERROR;
        else
        {
            floor = 0;
            return 0;
        }
    }

    if( time < _timestamps.front() || time > _timestamps.back() )
        return TIME_ERROR;
    else
    {
        floor = _binarySearch( _timestamps, time, 0, _timestamps.size() - 1 );
        return 0;
    }
}


template<typename T> size_t
VaporField::_binarySearch( const  std::vector<T>& vec, T val, 
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
VaporField::GetNumberOfTimesteps()
{
    return _timestamps.size();
}


int
VaporField::_getAGrid( size_t               timestep,
                       std::string&         varName,
                       const VAPoR::Grid**  gridpp  )
{
    // First check if we have the requested grid in our cache
    std::vector<double>           extMin, extMax;
    _params->GetBox()->GetExtents( extMin, extMax );
    int refLevel  = _params->GetRefinementLevel();
    int compLevel = _params->GetCompressionLevel();

    for( auto it = _recentGrids.cbegin(); it != _recentGrids.cend(); ++it )
        if( it->equals( timestep, varName, refLevel, compLevel, extMin, extMax ) )
        {   // We found it!
            *gridpp = it->realGrid;
            // Move this node to the front of the list
            if( it != _recentGrids.cbegin() )
                _recentGrids.splice( _recentGrids.cbegin(), _recentGrids, it );
            return 0;
        }

    // There's no such grid in our cache! Let's ask for it from the data manager,
    // and then keep it in our cache!
    VAPoR::Grid* grid = _datamgr->GetVariable( timestep, varName, refLevel, compLevel,
                                               extMin, extMax, true );
    if( grid == nullptr )
    {
        Wasp::MyBase::SetErrMsg("Not able to get a grid!");
        return GRID_ERROR;
    }
    *gridpp = grid;
    // Put it in our cache     
    _recentGrids.emplace_front( grid, timestep, varName, refLevel, compLevel,
                                extMin, extMax, _datamgr );
    if( _recentGrids.size() > _recentGridLimit )
        _recentGrids.pop_back();

    return 0;
}

/* Sometimes the container calls a default constructor like this.
 * We can delete this block of code if it doesn't cause problem for some time.
VaporField::RichGrid::RichGrid() :  realGrid( nullptr ),
                                    TS( 0 ),
                                    varName(),
                                    refinementLevel(0),
                                    compressionLevel(0),
                                    extMin(),
                                    extMax(),
                                    mgr( nullptr )
{}
*/

VaporField::RichGrid::RichGrid( const VAPoR::Grid* g, 
                                size_t currentTS,
                                const std::string& var, 
                                int refLevel, 
                                int compLevel,
                                const std::vector<double>& min, 
                                const std::vector<double>& max,
                                VAPoR::DataMgr* dm      )
                     :          realGrid( g ),
                                TS( currentTS ),
                                varName( var ),
                                refinementLevel( refLevel ),
                                compressionLevel( compLevel ),
                                extMin( min ),
                                extMax( max ),
                                mgr( dm )
{}

// Destructor
VaporField::RichGrid::~RichGrid()
{
    if( mgr && realGrid )
    {
        mgr->UnlockGrid( realGrid );
        delete realGrid;
    }
}

bool 
VaporField::RichGrid::equals( size_t currentTS, const std::string& var, 
                              int refLevel, int compLevel, 
                              const std::vector<double>& min, 
                              const std::vector<double>& max ) const
{
    if( currentTS == TS && var == varName && refLevel == refinementLevel &&
        compLevel == compressionLevel && min == extMin && max == extMax )
        return true;
    else
        return false;
}
