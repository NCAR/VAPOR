/* 
 * A stream that consists of many Particles.
 */ 

#ifndef STREAM_H
#define STREAM_H

#include "Particle.h"
#include <string>

namespace VAPoR
{
namespace flow
{
class Stream
{
public:
    std::vector<flow::Particle>     _particles;
    std::vector<std::string>        _propertyNames;

    // Constructors and destructor
    Stream();
   ~Stream();
};

};
};

#endif
