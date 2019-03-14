#pragma once

#include <vapor/Renderer.h>
#include <vapor/VolumeAlgorithm.h>
#include <glm/fwd.hpp>

using std::vector;
using std::string;

namespace VAPoR
{

class RENDER_API VolumeRenderer : public Renderer
{
public:
    VolumeRenderer(const ParamsMgr* pm,
                std::string&        winName,
                std::string&        dataSetName,
                std::string&        instName,
                DataMgr*            dataMgr );
    VolumeRenderer(const ParamsMgr* pm,
                   std::string&     winName,
                   std::string&     dataSetName,
                   std::string      paramsType,
                   std::string      classType,
                   std::string&     instName,
                   DataMgr*         dataMgr );
    ~VolumeRenderer();

    static std::string GetClassType()
    {
        return ("Volume2");
    }

protected:
    int _initializeGL();
    int _paintGL(bool fast);
    void _clearCache() {};
    
    int _loadData();
    void _loadTF();
    glm::vec3 _getVolumeScales() const;
    void _getExtents(glm::vec3 *dataMin, glm::vec3 *dataMax, glm::vec3 *userMin, glm::vec3 *userMax) const;
    
    VolumeAlgorithm *algorithm;
    
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int LUTTexture;
    unsigned int depthTexture;
    
    struct Cache {
        string var = "";
        size_t ts = -1;
        int refinement;
        int compression;
        
        MapperFunction *tf = nullptr;
        vector<double> mapRange;
        
        string algorithmName = "";
        
        bool needsUpdate;
    } cache;
};


};
