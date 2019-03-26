#ifndef VAPORFIELD_H
#define VAPORFIELD_H

#include "vapor/Field.h"
#include "vapor/Particle.h"
#include "vapor/DataMgr.h"
#include "vapor/FlowParams.h"
#include "vapor/Grid.h"

namespace flow
{
class VaporField : public Field
{
public:
    VaporField();
    virtual ~VaporField();

    //
    // Functions from class Field
    //
    virtual bool InsideVolume( float time, const glm::vec3& pos ) const;
    virtual int  GetVelocity(  float time, const glm::vec3& pos,     // input 
                               glm::vec3& vel ) const;               // output
    virtual int  GetScalar(    float time, const glm::vec3& pos,     // input 
                               float& val) const ;                   // output
    virtual int  GetNumberOfTimesteps() const;

    //
    // Functions for interaction with VAPOR components
    //
    void AssignDataManager( VAPoR::DataMgr*    dmgr );
    void UpdateParams(const VAPoR::FlowParams* p );

    //  
    // Find one index whose timestamp is just below a given time
    // I.e., _timestamps[floor] <= time 
    //  
    int LocateTimestamp( float   time,                        // Input
                         size_t& floor ) const;               // Output



private:
   
    // Member variables
    std::vector<float>          _timestamps;    // in ascending order
    VAPoR::DataMgr*             _datamgr;   
    const VAPoR::FlowParams*    _params;


    // Member functions
    template< typename T > 
    size_t _binarySearch( const std::vector<T>& vec, T val, size_t begin, size_t end ) const;

    // _getAGrid will use the cached params, _params, to generate grids. 
    int  _getAGrid( int               timestep,         // Input
                    std::string&      varName,          // Input
                    VAPoR::Grid**     gridpp  ) const;  // Output
};
};

#endif
