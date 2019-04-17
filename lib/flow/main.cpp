#include <iostream>
#include <cstdlib>
#include <list>
#include <forward_list>
#include <deque>
#include <stack>
#include <queue>
#include <vector>

#include "vapor/Particle.h"
#include "vapor/Advection.h"
#include "vapor/OceanField.h"

using namespace flow;

int main( int argc, char** argv )
{
/*
    int numOfSeeds = 5;
    std::vector<Particle> seeds( numOfSeeds );
    
    seeds[0].location = glm::vec3( 0.65f, 0.65f, 0.1f );
    seeds[1].location = glm::vec3( 0.3f, 0.3f, 0.1f );
    for( int i = 2; i < numOfSeeds; i++ )
        seeds[i].location = glm::vec3( float(i + 1) / float(numOfSeeds + 1), 0.0f, 0.0f );

    OceanField    f;
    Advection     a;
    a.SetBaseStepSize( 0.5f );
    
    a.UseField( &f );
    a.UseSeedParticles( seeds );

    int steps = std::atoi( argv[1] );
    for( int i = 0; i < steps; i++ )
        a.Advect( flow::Advection::RK4 );

    std::string filename( "streams.dat" );
    a.OutputStreamsGnuplot( filename );

    Particle p;
    int numOfProp = 10;
    for( int i = 0; i < numOfProp; i++ )
        p.AttachProperty( float(i) );

    for( int i = 0; i < p.GetNumOfProperties(); i++ )
    {
        std::cout << p.RetrieveProperty( i ) << std::endl;
    }
*/

    std::string line("  0.01,  1.0,1.1, 2.2,oijoi, ");
    line.push_back(',');
    size_t start = 0;
    size_t end   = line.find( ',' );
    std::vector<float> values;
    while( end != std::string::npos && values.size() < 4 )
    {
        auto str = line.substr( start, end - start );
        float val;
        try{ val = std::stof(str); }
        catch( const std::invalid_argument& e )
        {
            std::cout << "bad conversion" << std::endl;
            break;
        }
        values.push_back( val );
        start = end + 1;
        end = line.find(',', start );
    }

    for( auto e : values )
        std::cout << e << std::endl;
}
