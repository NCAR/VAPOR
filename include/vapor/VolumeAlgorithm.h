#pragma once

#include <vapor/Grid.h>
#include <vapor/ShaderManager.h>
#include <vector>
#include <string>

namespace VAPoR {
    
    struct GLManager;
    
    class VolumeAlgorithm {
    public:
        VolumeAlgorithm(GLManager *gl);
        virtual ~VolumeAlgorithm() {}
        virtual int LoadData(const Grid *grid) = 0;
        virtual ShaderProgram *GetShader(ShaderManager *sm) = 0;
        
        static const std::vector<std::string> &GetAlgorithmNames();
        static VolumeAlgorithm *NewAlgorithm(const std::string &name, GLManager *gl);
        
    private:
        static const std::vector<std::string> _algorithmNames;
        
    protected:
        GLManager *_glManager;
    };
    
}
