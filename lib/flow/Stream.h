/* 
 * A stream that consists of many Particles.
 */ 

#ifndef STREAM_H
#define STREAM_H

#include "Particle.h"
#include <string>
#include <map>

namespace flow
{
class Stream
{
private:
    std::vector<Particle>   _particles;
    size_t                  _startT, _finishT;
    
    // A Stream could optionally have multiple properties
    std::map< std::string, std::vector<float> >     properties;

public:
    // Constructors and destructor
    Stream();
   ~Stream();
};

};

#endif
