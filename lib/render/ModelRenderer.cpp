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
#include <vapor/VAssert.h>

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
    
    if (file != _cachedFile) {
        rc = _scene.Load(file);
        if (rc < 0)
            return rc;
        _cachedFile = file;
        glm::vec3 center = _scene.Center();
        rp->GetTransform()->SetOrigin({center.x, center.y, center.z});
    }
    
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
    
    _scene.Render(_glManager, 0);
    lgl->DisableLighting();
    
    /*
    glm::vec3 min = _model.BoundsMin();
    glm::vec3 max = _model.BoundsMax();
    
    printf("Model Min = [%f, %f, %f]\n", min.x, min.y, min.z);
    printf("Model Max = [%f, %f, %f]\n", max.x, max.y, max.z);
    
    lgl->DisableLighting();
    
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
     */
    
    return rc;
}

int ModelRenderer::_initializeGL()
{
    return 0;
}

void ModelRenderer::Model::renderNode(GLManager *gl, const aiNode *nd)
{
    LegacyGL *lgl = gl->legacy;
    MatrixManager *mm = gl->matrixManager;
    
    mm->PushMatrix();
    mm->SetCurrentMatrix(mm->GetCurrentMatrix() * getMatrix(nd));
    
    for (int m = 0; m < nd->mNumMeshes; m++) {
        const aiMesh *mesh = _scene->mMeshes[nd->mMeshes[m]];
        const bool hasNormals = mesh->HasNormals();
        const bool hasColor = mesh->GetNumColorChannels() > 0;
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
                if (hasColor)
                    lgl->Color3fv (&mesh->mColors[0][face->mIndices[v]].r);
                if (hasNormals)
                    lgl->Normal3fv(&mesh->mNormals[face->mIndices[v]].x);
                lgl->Vertex3fv(&mesh->mVertices[face->mIndices[v]].x);
            }
        }
        lgl->End();
    }
    
    for (int c = 0; c < nd->mNumChildren; c++)
        renderNode(gl, nd->mChildren[c]);
    
    mm->PopMatrix();
}

void ModelRenderer::Model::calculateBounds(const aiNode *nd, glm::mat4 transform)
{
    transform *= getMatrix(nd);
    
    for (int m = 0; m < nd->mNumMeshes; m++) {
        const aiMesh *mesh = _scene->mMeshes[nd->mMeshes[m]];
        
        for (int f = 0; f < mesh->mNumFaces; f++) {
            const aiFace *face = &mesh->mFaces[f];
            
            if (face->mNumIndices != 3)
                continue;
            
            for (int v = 0; v < face->mNumIndices; v++) {
                glm::vec3 gv = glm::make_vec3(&mesh->mVertices[face->mIndices[v]].x);
                gv = transform * glm::vec4(gv, 1.0f);
                _min = glm::min(_min, gv);
                _max = glm::max(_max, gv);
            }
        }
    }
    
    for (int c = 0; c < nd->mNumChildren; c++)
        calculateBounds(nd->mChildren[c], transform);
}

glm::mat4 ModelRenderer::Model::getMatrix(const aiNode *nd)
{
    // Ignore root transform. This is created by assimp to change the up axis
    if (nd == _scene->mRootNode)
        return glm::identity<glm::mat4>();
    
    aiMatrix4x4 m = nd->mTransformation;
    m.Transpose();
    return glm::make_mat4((float*)&m);
}

void ModelRenderer::Model::Render(GLManager *gl)
{
    gl->legacy->DisableTexture();
    
    renderNode(gl, _scene->mRootNode);
}

int ModelRenderer::Model::Load(const std::string &path)
{
    if (!FileUtils::Exists(path)) {
        MyBase::SetErrMsg("File not found \"%s\"", path.c_str());
        return -1;
    }
    
    if (_importer.GetScene())
        _importer.FreeScene();
    
    _scene = _importer.ReadFile(path,
                              aiProcessPreset_TargetRealtime_Quality |
                              aiProcess_Triangulate
                              );
    
    if (!_scene) {
        MyBase::SetErrMsg("3D File Error: %s", _importer.GetErrorString());
        return -1;
    }
    
    _min = glm::vec3(FLT_MAX);
    _max = glm::vec3(FLT_MIN);
    calculateBounds(_scene->mRootNode);
    
    return 0;
}

int ModelRenderer::Scene::Load(const std::string &path)
{
    _keyframes.clear();
    _models.clear();
    
    if (FileUtils::Extension(path) == "vms") {
        
    } else {
        Model *model = new Model;
        int rc = model->Load(path);
        if (rc < 0)
            return rc;
        _models[path] = unique_ptr<Model>(model);
        
        ModelInstance defaultInstance;
        defaultInstance.path = path;
        _keyframes[0] = {defaultInstance};
    }
    
    return 0;
}

void ModelRenderer::Scene::Render(GLManager *gl, const int ts)
{
    MatrixManager *mm = gl->matrixManager;
    const vector<ModelInstance> instances = getInstances(ts);
    
    for (const auto &instance : instances) {
        mm->PushMatrix();
        mm->Translate(instance.translation.x, instance.translation.y, instance.translation.z);
        _models[instance.path]->Render(gl);
        mm->PopMatrix();
    }
}

glm::vec3 ModelRenderer::Scene::Center() const
{
    glm::vec3 accum(0.f);
    for (const auto & it: _models)
        accum += it.second->Center();
    if (!_models.empty())
        accum /= (float)_models.size();
    return accum;
}

std::vector<ModelRenderer::Scene::ModelInstance> ModelRenderer::Scene::getInstances(const int ts) const
{
    VAssert(ts >= 0);
    int lastValidFrame = -1;
    for (auto frame : _keyframes) {
        const int frameTime = frame.first;
        if (frameTime == ts)
            return frame.second;
        if (frameTime < ts && frameTime > lastValidFrame)
            lastValidFrame = frameTime;
    }
    if (lastValidFrame != -1)
        return _keyframes.at(lastValidFrame);
    else
        return vector<ModelInstance>();
}
