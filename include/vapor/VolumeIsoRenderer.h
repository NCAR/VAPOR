#pragma once

#include <vapor/VolumeRenderer.h>
#include <vapor/VolumeAlgorithm.h>
#include <glm/fwd.hpp>

using std::vector;
using std::string;

namespace VAPoR
{
    
    //! \class VolumeIsoRenderer
    //! \ingroup Public_Render
    //!
    //! \brief Isosurface renderer
    //!
    //! \author Stanislaw Jaroszynski
    //! \date Feburary, 2019
    //!
    //! Since Vapor requires a separate class for each renderer, this
    //! class is a copy of the VolumeRenderer class but it only gives access
    //! to the isosurface rendering algorithms

class RENDER_API VolumeIsoRenderer : public VolumeRenderer
{
public:
    VolumeIsoRenderer( const ParamsMgr*    pm,
                std::string&        winName,
                std::string&        dataSetName,
                std::string&        instName,
                DataMgr*            dataMgr );
    ~VolumeIsoRenderer();

    static std::string GetClassType()
    {
        return ("IsoSurface");
    }
    
protected:
    virtual bool _usingColorMapData() const;
    virtual void _setShaderUniforms(const ShaderProgram *shader, const bool fast) const;
    virtual std::string _getDefaultAlgorithmForGrid(const Grid *grid) const;
    virtual void _getLUTFromTF(const MapperFunction *tf, float *LUT) const;
    
public:
    int OSPRayUpdate(OSPModel world);
    void OSPRayDelete(OSPModel world);
    
protected:
    int OSPRayLoadTF();
    void OSPRayAddObjectToWorld(OSPModel world);
    void OSPRayRemoveObjectFromWorld(OSPModel world);
    
    OSPMaterial _ospMaterial = nullptr;
    OSPGeometry _ospIsoSurfaces = nullptr;
};


};
