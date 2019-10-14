//************************************************************************
//                                                                       *
//                          Copyright (C)  2018                          *
//            University Corporation for Atmospheric Research            *
//                          All Rights Reserved                          *
//                                                                       *
//************************************************************************/
//
//  File:   ModelRenderer.cpp
//
//  Author: Stas Jaroszynski
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:   March 2018
//
//  Description:
//          Definition of ModelRenderer
//
#pragma once

#include <GL/glew.h>
#ifdef Darwin
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <vapor/Renderer.h>
#include <vapor/ModelParams.h>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <memory>

namespace VAPoR {

class DataMgr;
    
//! \class ModelRenderer
//! \brief Class that draws the contours (contours) as specified by IsolineParams
//! \author Stas Jaroszynski
//! \version 1.0
//! \date March 2018
class RENDER_API ModelRenderer : public Renderer
{

public:

	ModelRenderer(const ParamsMgr* pm,
							string winName,
							string dataSetName,
							string instName,
							DataMgr* dataMgr);

	virtual ~ModelRenderer();

	static string GetClassType() {
		return("Model");
	}
	
	//! \copydoc Renderer::_initializeGL()
	virtual int	_initializeGL();
	//! \copydoc Renderer::_paintGL()
    virtual int _paintGL(bool fast);
    virtual void _clearCache() {}
    
private:
    class Model {
        Assimp::Importer _importer;
        const aiScene *_scene;
        glm::vec3 _min, _max;
        
        void renderNode(GLManager *gl, const aiNode *nd);
        void calculateBounds(const aiNode *nd, glm::mat4 transform = glm::mat4(1.0f));
        glm::mat4 getMatrix(const aiNode *nd);
        
    public:
        
        ~Model() {
            printf("Deleting Model!\n");
            
        }
        void Render(GLManager *gl);
        int Load(const std::string &path);
        glm::vec3 BoundsMin() const { return _min; }
        glm::vec3 BoundsMax() const { return _max; }
        glm::vec3 Center() const { return (_min + _max) / 2.f; }
    };
    
    class Scene {
        struct ModelInstance {
            std::string path;
            glm::vec3 translation = glm::vec3(0.f);
            glm::vec3 rotation = glm::vec3(0.f);
            glm::vec3 scale = glm::vec3(1.f);
        };
        
        std::map<int, std::vector<ModelInstance>> _keyframes;
        std::map<std::string, std::unique_ptr<Model>> _models;
        
    public:
        int Load(const std::string &path);
        void Render(GLManager *gl, const int ts = 0);
        glm::vec3 Center() const;
        
    private:
        std::vector<ModelInstance> getInstances(const int ts) const;
    };
 
    Scene _scene;
    std::string _cachedFile;
    
    void _renderNode(const aiNode *node);
};
    
};
