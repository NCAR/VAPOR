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
#include <vapor/XmlNode.h>
#include <assimp/postprocess.h>
#include <assimp/version.h>

#warning vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
#warning Third party libraries updated to assimp v5. need to restore assimp_backup and remove assimp.5.*
#warning ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
    printf("ASSIMP %i.%i.%i\n", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
}

ModelRenderer::~ModelRenderer()
{
}

void printSpacing(int depth) {
    for (int i = 0; i < depth; i++)
        printf("  ");
}

void printNode(XmlNode *n, int depth=0)
{
    printSpacing(depth);
    printf("<%s", n->Tag().c_str());
    for (auto attr : n->Attrs())
        printf(" %s=\"%s\"", attr.first.c_str(), attr.second.c_str());
    
    if (n->GetNumChildren() == 0) {
        printf(" />\n");
        return;
    }
    printf(">\n");
    
    for (int i = 0; i < n->GetNumChildren(); i++)
        printNode(n->GetChild(i), depth+1);
    
    printSpacing(depth);
    printf("</%s>\n", n->Tag().c_str());
}

int ModelRenderer::_paintGL(bool fast)
{
    RenderParams *rp = GetActiveParams();
    int rc = 0;
    const std::string file = rp->GetValueString("file", "");
    
    if (FileUtils::Extension(file) == "vms" && FileUtils::Exists(file)) {
        XmlParser parser;
        XmlNode node;
        rc = parser.LoadFromFile(&node, file);
        if (rc < 0) return rc;
        
        printf("==============================\n");
        printNode(&node);
        
        return 0;
    }
    
    if (file != _cachedFile)
        rc = loadFile(file);
    if (rc < 0)
        return rc;
    _cachedFile = file;
    
    LegacyGL *lgl = _glManager->legacy;
    
//    if (fast)
//        return 0;
    
    MatrixManager *mm = _glManager->matrixManager;
    mm->MatrixModeModelView();
    
//    LegacyGL *lgl = _glManager->legacy;
    lgl->EnableLighting();
    lgl->DisableTexture();
    
    ViewpointParams *viewpointParams = _paramsMgr->GetViewpointParams(_winName);
    Viewpoint *viewpoint = viewpointParams->getCurrentViewpoint();
    double m[16];
    double cameraPos[3], cameraUp[3], cameraDir[3];
    _glManager->matrixManager->GetDoublev(MatrixManager::Mode::ModelView, m);
    viewpoint->ReconstructCamera(m, cameraPos, cameraUp, cameraDir);
    
    lgl->Color3f(1, 1, 1);
    float lightDir[3] = {(float)cameraDir[0], (float)cameraDir[1], (float)cameraDir[2]};
    lgl->LightDirectionfv(lightDir);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    
    min.x = FLT_MAX;
    min.y = FLT_MAX;
    min.z = FLT_MAX;
    max.x = FLT_MIN;
    max.y = FLT_MIN;
    max.z = FLT_MIN;
    
    n = 0;
    renderNode(scene->mRootNode);
    printf("N Verts = %li\n", n);
    
    printf("Model Min = [%f, %f, %f]\n", min.x, min.y, min.z);
    printf("Model Max = [%f, %f, %f]\n", max.x, max.y, max.z);
    
    lgl->DisableLighting();
    return rc;
    
    lgl->Begin(GL_LINES);
    lgl->Color3f(1, 0, 0);
    lgl->Vertex3f(min.x, min.y, min.z);
    lgl->Vertex3f(max.x, min.y, min.z);
    
    lgl->Color3f(0, 1, 0);
    lgl->Vertex3f(min.x, min.y, min.z);
    lgl->Vertex3f(min.x, max.y, min.z);
    
    lgl->Color3f(0, 0, 1);
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
    LegacyGL *lgl = _glManager->legacy;
    MatrixManager *mm = _glManager->matrixManager;
    
    aiMatrix4x4 m;
    if (nd != scene->mRootNode)
        m = nd->mTransformation;
    m.Transpose();
    
    mm->PushMatrix();
    mm->SetCurrentMatrix(mm->GetCurrentMatrix() * glm::make_mat4((float*)&m));
    
    for (int m = 0; m < nd->mNumMeshes; m++) {
        const aiMesh *mesh = scene->mMeshes[nd->mMeshes[m]];
        if (mesh->HasNormals())
            lgl->EnableLighting();
        else
            lgl->DisableLighting();
        
        lgl->Begin(GL_TRIANGLES);
        for (int f = 0; f < mesh->mNumFaces; f++) {
            const aiFace *face = &mesh->mFaces[f];
            
            if (face->mNumIndices != 3)
                continue;
            
            for (int v = 0; v < face->mNumIndices; v++) {
                n++;
                const aiVector3D &vertex = mesh->mVertices[face->mIndices[v]];
                const glm::vec3 gv(vertex.x, vertex.y, vertex.z);
                min = glm::min(min, gv);
                max = glm::max(max, gv);
                if (mesh->GetNumColorChannels() > 0)
                    lgl->Color3fv (&mesh->mColors[0][face->mIndices[v]].r);
                if (mesh->HasNormals())
                    lgl->Normal3fv(&mesh->mNormals[face->mIndices[v]].x);
                lgl->Vertex3fv(&mesh->mVertices[face->mIndices[v]].x);
            }
        }
        lgl->End();
    }
    
    for (int c = 0; c < nd->mNumChildren; c++)
        renderNode(nd->mChildren[c]);
    
    mm->PopMatrix();
}

int ModelRenderer::loadFile(const std::string &path)
{
    if (!FileUtils::Exists(path)) {
        MyBase::SetErrMsg("File not found \"%s\"", path.c_str());
        return -1;
    }
    
    if (importer.GetScene())
        importer.FreeScene();
    
    scene = importer.ReadFile(path,
                              aiProcessPreset_TargetRealtime_Quality |
                              aiProcess_Triangulate
                              );
    
    if (!scene) {
        MyBase::SetErrMsg("3D File Error: %s", importer.GetErrorString());
        return -1;
    }
    return 0;
}
