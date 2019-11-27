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
#include <string>

namespace VAPoR {

class DataMgr;

//! \class ModelRenderer
//! \brief Class that draws the contours (contours) as specified by IsolineParams
//! \author Stas Jaroszynski
//! \version 1.0
//! \date March 2018
class RENDER_API ModelRenderer : public Renderer {
public:
    ModelRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr);

    static string GetClassType() { return ("Model"); }

    //! \copydoc Renderer::_initializeGL()
    virtual int _initializeGL();
    //! \copydoc Renderer::_paintGL()
    virtual int  _paintGL(bool fast);
    virtual void _clearCache() {}

private:
    class Model {
        Assimp::Importer _importer;
        const aiScene *  _scene;
        glm::vec3        _min, _max;

        void      renderNode(GLManager *gl, const aiNode *nd) const;
        void      calculateBounds(const aiNode *nd, glm::mat4 transform = glm::mat4(1.0f));
        glm::mat4 getMatrix(const aiNode *nd) const;

    public:
        void      Render(GLManager *gl) const;
        void      DrawBoundingBox(GLManager *gl) const;
        int       Load(const std::string &path);
        glm::vec3 BoundsMin() const { return _min; }
        glm::vec3 BoundsMax() const { return _max; }
        glm::vec3 Center() const { return (_min + _max) / 2.f; }
    };

    class Scene {
        struct ModelInstance {
            std::string name;
            std::string file;
            glm::vec3   translate = glm::vec3(0.f);
            glm::vec3   rotate = glm::vec3(0.f);
            glm::vec3   scale = glm::vec3(1.f);
            glm::vec3   origin = glm::vec3(0.f);
        };

        std::vector<ModelInstance>                _instances;
        std::map<int, std::vector<ModelInstance>> _keyframes;
        std::map<std::string, Model *>            _models;

    public:
        ~Scene();
        int       Load(const std::string &path);
        void      Render(GLManager *gl, const int ts = 0);
        glm::vec3 Center() const;

    private:
        std::vector<ModelInstance> getInstances(const int ts) const;
        int                        createSceneFromModelFile(const std::string &path);
        int                        loadSceneFile(const std::string &path);
        int                        handleInstanceNode(XmlNode *node, ModelInstance *instance);
        int                        handleTimeNode(XmlNode *node);
        int                        handleVectorNode(XmlNode *node, glm::vec3 *v);
        int                        handleFloatAttribute(XmlNode *node, const std::string &name, float *f);
        int                        parseIntString(const std::string &str, int *i) const;
        ModelInstance              getInitInstance(const std::string &name) const;
        bool                       doesInstanceExist(const std::string &name) const;
        bool                       isModelCached(const std::string &file) const;
    };

    Scene       _scene;
    std::string _cachedFile;

    void _renderNode(const aiNode *node);
};

};    // namespace VAPoR
