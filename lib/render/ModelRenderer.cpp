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
#include <vapor/LegacyGL.h>
#include <vapor/FileUtils.h>
#include <assimp/postprocess.h>

using namespace VAPoR;

static RendererRegistrar<ModelRenderer> registrar(
                                                    ModelRenderer::GetClassType(), ModelParams::GetClassType()
                                                    );

ModelRenderer::ModelRenderer(const ParamsMgr* pm, string winName,
                                 string dataSetName, string instName,
                                 DataMgr* dataMgr)
: Renderer(pm, winName, dataSetName, ModelParams::GetClassType(),
           ModelRenderer::GetClassType(), instName, dataMgr)
{
    const string path = "/Users/stas/Downloads/Windmill/nrel5MW-small.stl";
    
    assert(FileUtils::Exists(path));
    scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality);
    printf("Importer diag: %s\n", importer.GetErrorString());
}

ModelRenderer::~ModelRenderer()
{
}

bool PRINT_VERTS = false;

int ModelRenderer::_paintGL(bool fast)
{
    LegacyGL *lgl = _glManager->legacy;
    
    
//    if (fast)
//        return 0;
    int rc = 0;
    
    MatrixManager *mm = _glManager->matrixManager;
    mm->MatrixModeModelView();
    
//    LegacyGL *lgl = _glManager->legacy;
    lgl->DisableLighting();
    lgl->DisableTexture();
    
    lgl->Color3f(1, 1, 1);
    
    min.x = FLT_MAX;
    min.y = FLT_MAX;
    min.z = FLT_MAX;
    max.x = FLT_MIN;
    max.y = FLT_MIN;
    max.z = FLT_MIN;
    
    n = 0;
    PRINT_VERTS = true;
    renderNode(scene->mRootNode);
    PRINT_VERTS = false;
    printf("N Verts = %li\n", n);
    
    printf("Model Min = [%f, %f, %f]\n", min.x, min.y, min.z);
    printf("Model Max = [%f, %f, %f]\n", max.x, max.y, max.z);
    
    lgl->Begin(GL_LINES);
    lgl->Vertex3f(min.x, min.y, min.z);
    lgl->Vertex3f(max.x, min.y, min.z);
    
    lgl->Vertex3f(min.x, min.y, min.z);
    lgl->Vertex3f(min.x, max.y, min.z);
    
    lgl->Vertex3f(min.x, min.y, min.z);
    lgl->Vertex3f(min.x, min.y, max.z);
    lgl->End();
    
    return rc;
}

int ModelRenderer::_initializeGL()
{
    return 0;
}

void ModelRenderer::renderNode(const aiNode *nd)
{
//    LegacyGL *lgl = _glManager->legacy;
//    MatrixManager *mm = _glManager->matrixManager;
    
    aiMatrix4x4 m = nd->mTransformation;
    m.Transpose();
    
//    mm->PushMatrix();
//    mm->SetCurrentMatrix(mm->GetCurrentMatrix() * glm::make_mat4((float*)&m));
    
    for (int m = 0; m < nd->mNumMeshes; m++) {
        const aiMesh *mesh = scene->mMeshes[nd->mMeshes[m]];
        
        for (int f = 0; f < mesh->mNumFaces; f++) {
            const aiFace *face = &mesh->mFaces[f];
            int mode;
            switch (face->mNumIndices) {
                case 1:  mode = GL_POINTS;    break;
                case 2:  mode = GL_LINES;     break;
                case 3:  mode = GL_TRIANGLES; break;
                default: mode = GL_POINTS;    assert(0);
            }
            mode = GL_POINTS;
//            lgl->Begin(GL_POINTS);
            
            for (int v = 0; v < face->mNumIndices; v++) {
                n++;
                const aiVector3D &vertex = mesh->mVertices[face->mIndices[v]];
                const glm::vec3 gv(vertex.x, vertex.y, vertex.z);
                min = glm::min(min, gv);
                max = glm::max(max, gv);
//                lgl->Vertex3fv(&mesh->mVertices[face->mIndices[v]].x);
            }
            
//            lgl->End();
        }
    }
    
    for (int c = 0; c < nd->mNumChildren; c++)
        renderNode(nd->mChildren[c]);
    
//    mm->PopMatrix();
}
