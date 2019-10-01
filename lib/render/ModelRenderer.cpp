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
#include "vapor/GLManager.h"

using namespace VAPoR;

static RendererRegistrar<ModelRenderer> registrar(
                                                    ModelRenderer::GetClassType(), ModelParams::GetClassType()
                                                    );

ModelRenderer::ModelRenderer(const ParamsMgr* pm, string winName,
                                 string dataSetName, string instName,
                                 DataMgr* dataMgr)
: Renderer(pm, winName, dataSetName, ModelParams::GetClassType(),
           ModelRenderer::GetClassType(), instName, dataMgr){}

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

