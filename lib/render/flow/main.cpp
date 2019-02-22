#include <iostream>
#include <cstdlib>
#include <list>
#include <forward_list>
#include <deque>
#include <stack>
#include <queue>
#include <vector>

#include "Particle.h"
#include "Advection.h"
#include "OceanField.h"

using namespace flow;

int main( int argc, char** argv )
{
    int numOfSeeds = 5;
    std::vector<Particle> seeds( numOfSeeds );
    
    for( int i = 0; i < numOfSeeds; i++ )
        seeds[i].location = glm::vec3( float(i + 1) / float(numOfSeeds + 1), 0.0f, 0.0f );

    OceanField    f;
    Advection     a;
    
    a.UseVelocityField( &f );
    a.UseSeedParticles( seeds );

    int steps = std::atoi( argv[1] );
    for( int i = 0; i < steps; i++ )
        a.Advect( 0.02 );

    std::string filename( "streams.dat" );
    a.OutputStreamsGnuplot( filename );
}
