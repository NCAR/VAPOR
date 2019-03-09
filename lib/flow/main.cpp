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
}
