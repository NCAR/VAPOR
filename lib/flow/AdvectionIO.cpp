#include <iostream>
#include <fstream>
#include "vapor/AdvectionIO.h"

auto flow::OutputGnuplotNumSteps( const Advection*  adv,
                                  const char*       filename,
                                  size_t            numSteps,
                                  bool              append ) -> int
{
    std::FILE* f = nullptr;
    if( append ) {
        f = std::fopen( filename, "a" );
    }
    else {
        f = std::fopen( filename, "w" );
    }
    if( f == nullptr )
        return FILE_ERROR;

    auto propertyNames = adv->GetPropertyVarNames();

    // Write the header
    if( !append ) {
        std::fprintf( f, "%s%s", "# ID,  X-position,  Y-position,  Z-position,  Time,   ",
                                 adv->GetValueVarName().c_str() );

        for( auto& n : propertyNames )
            std::fprintf( f, ",  %s", n.c_str() );
        std::fprintf( f, "\n" );
    }

    // Write the trajectories
    for( size_t s_idx = 0; s_idx < adv->GetNumberOfStreams(); s_idx++ ) {
        const auto& stream = adv->GetStreamAt( s_idx );

        size_t step = 0;
        for( const auto& p : stream ) {
            if( !p.IsSpecial() ) {
                std::fprintf( f, "%lu, %f, %f, %f, %f, %f", s_idx, p.location.x, p.location.y,
                                  p.location.z, p.time, p.value );

                auto props = p.GetPropertyList();
                assert( props.second == propertyNames.size() ); // sanity check
                for( const auto& val : props.first )
                    std::fprintf( f, ", %f", val );

                std::fprintf( f, "\n" ); // end of one line
                step++;
            }
            if( step > numSteps )   // when numSteps + 1 particles are printed.
                break;
        }
    }

    std::fclose( f );

    return 0;

}


auto flow::OutputGnuplotMaxTime( const Advection*  adv,
                                 const char*       filename,
                                 float             maxTime,
                                 bool              append ) -> int
{
    std::FILE* f = nullptr;
    if( append ) {
        f = std::fopen( filename, "a" );
    }
    else {
        f = std::fopen( filename, "w" );
    }
    if( f == nullptr )
        return FILE_ERROR;

    auto propertyNames = adv->GetPropertyVarNames();

    // Write the header
    if( !append ) {
        std::fprintf( f, "%s%s", "# ID,  X-position,  Y-position,  Z-position,  Time,   ",
                                 adv->GetValueVarName().c_str() );

        for( auto& n : propertyNames )
            std::fprintf( f, ",  %s", n.c_str() );
        std::fprintf( f, "\n" );
    }

    // Write the trajectories
    for( size_t s_idx = 0; s_idx < adv->GetNumberOfStreams(); s_idx++ ) {
        const auto& stream = adv->GetStreamAt( s_idx );

        for( const auto& p : stream ) {
            
            if( p.time > maxTime )
                break;

            if( !p.IsSpecial() ) {
                std::fprintf( f, "%lu, %f, %f, %f, %f, %f", s_idx, p.location.x, p.location.y,
                                  p.location.z, p.time, p.value );

                auto props = p.GetPropertyList();
                assert( props.second == propertyNames.size() ); // sanity check
                for( const auto& val : props.first )
                    std::fprintf( f, ", %f", val );

                std::fprintf( f, "\n" ); // end of one line
            }
        }
    }

    std::fclose( f );

    return 0;
}
