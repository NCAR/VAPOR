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
//          Implementation of ModelRenderer
//

#include <string>

#include <vapor/glutil.h>    // Must be included first!!!

#include <vapor/ModelRenderer.h>
#include <vapor/ShaderManager.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vapor/GLManager.h>
#include <vapor/FileUtils.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

using namespace VAPoR;

const aiScene *scene;

static RendererRegistrar<ModelRenderer> registrar(
                                                    ModelRenderer::GetClassType(), ModelParams::GetClassType()
                                                    );

ModelRenderer::ModelRenderer(const ParamsMgr* pm, string winName,
                                 string dataSetName, string instName,
                                 DataMgr* dataMgr)
: Renderer(pm, winName, dataSetName, ModelParams::GetClassType(),
           ModelRenderer::GetClassType(), instName, dataMgr)
{
    Assimp::Importer importer;
    const string path = "/Users/stas/Downloads/Windmill/nrel5MW-small.stl";
    
    scene = importer.ReadFile(path, 0);
    if (!scene) {
        printf("ERROR %s\n", importer.GetErrorString());
    }
}

ModelRenderer::~ModelRenderer()
{
}

int ModelRenderer::_paintGL(bool)
{
    int rc = 0;
    return rc;
}

int ModelRenderer::_initializeGL()
{
    return 0;
}

