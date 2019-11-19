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
#include <vapor/STLUtils.h>
#include <vapor/XmlNode.h>
#include <assimp/postprocess.h>
#include <assimp/version.h>
#include <vapor/VAssert.h>

using namespace VAPoR;

static RendererRegistrar<ModelRenderer> registrar(ModelRenderer::GetClassType(), ModelParams::GetClassType());

ModelRenderer::ModelRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, ModelParams::GetClassType(), ModelRenderer::GetClassType(), instName, dataMgr)
{
}

int ModelRenderer::_paintGL(bool fast)
{
    RenderParams *    rp = GetActiveParams();
    int               rc = 0;
    const std::string file = rp->GetValueString(ModelParams::FileTag, "");

    if (file != _cachedFile) {
        rc = _scene.Load(file);
        if (rc < 0) {
            _cachedFile = "";
            return rc;
        }
        _cachedFile = file;
        glm::vec3 center = _scene.Center();
        rp->GetTransform()->SetOrigin({center.x, center.y, center.z});
    }

    LegacyGL *lgl = _glManager->legacy;

    MatrixManager *mm = _glManager->matrixManager;
    mm->MatrixModeModelView();

    lgl->EnableLighting();
    lgl->DisableTexture();

    ViewpointParams *viewpointParams = _paramsMgr->GetViewpointParams(_winName);
    Viewpoint *      viewpoint = viewpointParams->getCurrentViewpoint();
    double           m[16];
    double           cameraPos[3], cameraUp[3], cameraDir[3];
    _glManager->matrixManager->GetDoublev(MatrixManager::Mode::ModelView, m);
    viewpoint->ReconstructCamera(m, cameraPos, cameraUp, cameraDir);

    lgl->Color3f(1, 1, 1);
    float lightDir[3] = {(float)cameraDir[0], (float)cameraDir[1], (float)cameraDir[2]};
    lgl->LightDirectionfv(lightDir);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    _scene.Render(_glManager, rp->GetCurrentTimestep());
    lgl->DisableLighting();

    return rc;
}

int ModelRenderer::_initializeGL() { return 0; }

void ModelRenderer::Model::renderNode(GLManager *gl, const aiNode *nd) const
{
    LegacyGL *     lgl = gl->legacy;
    MatrixManager *mm = gl->matrixManager;

    mm->PushMatrix();
    mm->SetCurrentMatrix(mm->GetCurrentMatrix() * getMatrix(nd));

    for (int m = 0; m < nd->mNumMeshes; m++) {
        const aiMesh *mesh = _scene->mMeshes[nd->mMeshes[m]];
        const bool    hasNormals = mesh->HasNormals();
        const bool    hasColor = mesh->GetNumColorChannels() > 0;
        if (mesh->HasNormals())
            lgl->EnableLighting();
        else
            lgl->DisableLighting();

        lgl->Begin(GL_TRIANGLES);
        for (int f = 0; f < mesh->mNumFaces; f++) {
            const aiFace *face = &mesh->mFaces[f];

            if (face->mNumIndices != 3) continue;

            for (int v = 0; v < face->mNumIndices; v++) {
                if (hasColor) lgl->Color3fv(&mesh->mColors[0][face->mIndices[v]].r);
                if (hasNormals) lgl->Normal3fv(&mesh->mNormals[face->mIndices[v]].x);
                lgl->Vertex3fv(&mesh->mVertices[face->mIndices[v]].x);
            }
        }
        lgl->End();
    }

    for (int c = 0; c < nd->mNumChildren; c++) renderNode(gl, nd->mChildren[c]);

    mm->PopMatrix();
}

void ModelRenderer::Model::calculateBounds(const aiNode *nd, glm::mat4 transform)
{
    transform *= getMatrix(nd);

    for (int m = 0; m < nd->mNumMeshes; m++) {
        const aiMesh *mesh = _scene->mMeshes[nd->mMeshes[m]];

        for (int f = 0; f < mesh->mNumFaces; f++) {
            const aiFace *face = &mesh->mFaces[f];

            if (face->mNumIndices != 3) continue;

            for (int v = 0; v < face->mNumIndices; v++) {
                glm::vec3 gv = glm::make_vec3(&mesh->mVertices[face->mIndices[v]].x);
                gv = transform * glm::vec4(gv, 1.0f);
                _min = glm::min(_min, gv);
                _max = glm::max(_max, gv);
            }
        }
    }

    for (int c = 0; c < nd->mNumChildren; c++) calculateBounds(nd->mChildren[c], transform);
}

glm::mat4 ModelRenderer::Model::getMatrix(const aiNode *nd) const
{
    // Ignore root transform. This is created by assimp to change the up axis
    if (nd == _scene->mRootNode) return glm::identity<glm::mat4>();

    aiMatrix4x4 m = nd->mTransformation;
    m.Transpose();
    return glm::make_mat4((float *)&m);
}

void ModelRenderer::Model::Render(GLManager *gl) const
{
    gl->legacy->Color3f(1, 1, 1);
    gl->legacy->DisableTexture();
    VAssert(_scene);
    renderNode(gl, _scene->mRootNode);
}

void ModelRenderer::Model::DrawBoundingBox(GLManager *gl) const
{
    LegacyGL *lgl = gl->legacy;
    lgl->DisableLighting();

    lgl->Begin(GL_LINES);
    lgl->Color3f(1, 0, 0);
    lgl->Vertex3f(_min.x, _min.y, _min.z);
    lgl->Vertex3f(_max.x, _min.y, _min.z);

    lgl->Color3f(0, 1, 0);
    lgl->Vertex3f(_min.x, _min.y, _min.z);
    lgl->Vertex3f(_min.x, _max.y, _min.z);

    lgl->Color3f(0, 0, 1);
    lgl->Vertex3f(_min.x, _min.y, _min.z);
    lgl->Vertex3f(_min.x, _min.y, _max.z);
    lgl->End();
}

int ModelRenderer::Model::Load(const std::string &path)
{
    if (!FileUtils::Exists(path)) {
        MyBase::SetErrMsg("File not found \"%s\"", path.c_str());
        return -1;
    }

    if (_importer.GetScene()) _importer.FreeScene();

    _scene = _importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality | aiProcess_Triangulate);

    if (!_scene) {
        MyBase::SetErrMsg("3D File Error: %s", _importer.GetErrorString());
        return -1;
    }

    _min = glm::vec3(FLT_MAX);
    _max = glm::vec3(FLT_MIN);
    calculateBounds(_scene->mRootNode);

    return 0;
}

ModelRenderer::Scene::~Scene()
{
    for (auto it : _models) delete it.second;
}

int ModelRenderer::Scene::Load(const std::string &path)
{
    for (auto it : _models) delete it.second;
    _keyframes.clear();
    _models.clear();
    _instances.clear();

    int rc;
    if (FileUtils::Extension(path) == "vms")
        rc = loadSceneFile(path);
    else
        rc = createSceneFromModelFile(path);

    return rc;
}

void ModelRenderer::Scene::Render(GLManager *gl, const int ts)
{
    MatrixManager *              mm = gl->matrixManager;
    const vector<ModelInstance> &keyframe = getInstances(ts);

    for (const auto &instance : keyframe) {
        mm->PushMatrix();
        const glm::vec3 translate = instance.translate;
        const glm::vec3 rotate = instance.rotate;
        const glm::vec3 scale = instance.scale;
        const glm::vec3 origin = instance.origin;

        mm->Translate(translate.x, translate.y, translate.z);
        mm->Translate(origin.x, origin.y, origin.z);
        mm->Rotate(glm::radians(rotate.x), 1, 0, 0);
        mm->Rotate(glm::radians(rotate.y), 0, 1, 0);
        mm->Rotate(glm::radians(rotate.z), 0, 0, 1);
        mm->Scale(scale.x, scale.y, scale.z);
        mm->Translate(-origin.x, -origin.y, -origin.z);

        _models[instance.file]->Render(gl);
        mm->PopMatrix();
    }
}

glm::vec3 ModelRenderer::Scene::Center() const
{
    glm::vec3 accum(0.f);
    for (const auto &it : _models) accum += it.second->Center();
    if (!_models.empty()) accum /= (float)_models.size();
    return accum;
}

std::vector<ModelRenderer::Scene::ModelInstance> ModelRenderer::Scene::getInstances(const int ts) const
{
    VAssert(ts >= 0);
    int lastValidFrame = -1;
    for (auto frame : _keyframes) {
        const int frameTime = frame.first;
        if (frameTime == ts) return frame.second;
        if (frameTime < ts && frameTime > lastValidFrame) lastValidFrame = frameTime;
    }
    if (lastValidFrame != -1)
        return _keyframes.at(lastValidFrame);
    else
        return vector<ModelInstance>();
}

int ModelRenderer::Scene::createSceneFromModelFile(const std::string &path)
{
    Model *model = new Model;
    int    rc = model->Load(path);
    if (rc < 0) return rc;
    _models[path] = model;

    ModelInstance defaultInstance;
    defaultInstance.file = path;
    _keyframes[0] = {defaultInstance};

    return 0;
}

#define TAG_INSTANCE "instance_"
#define TAG_TIME     "time_"

int ModelRenderer::Scene::loadSceneFile(const std::string &sceneFilePath)
{
    XmlNode   root;
    XmlParser xmlParser;
    int       rc = xmlParser.LoadFromFile(&root, sceneFilePath);
    if (rc < 0) return rc;

    for (int i = 0; i < root.GetNumChildren(); i++) {
        XmlNode *node = root.GetChild(i);

        if (STLUtils::BeginsWith(node->Tag(), TAG_INSTANCE)) {
            ModelInstance instance;
            rc = handleInstanceNode(node, &instance);
        } else if (STLUtils::BeginsWith(node->Tag(), TAG_TIME)) {
            rc = handleTimeNode(node);
        } else {
            MyBase::SetErrMsg("Unknown tag \"%s\"", node->Tag().c_str());
            rc = -1;
        }

        if (rc < 0) {
            MyBase::SetErrMsg("%s ->", root.Tag().c_str());
            break;
        }
    }

    if (rc < 0) return rc;

    for (const ModelInstance &instance : _instances) {
        if (!isModelCached(instance.file)) {
            string filePath = instance.file;
            if (!FileUtils::IsPathAbsolute(filePath)) filePath = FileUtils::JoinPaths({FileUtils::Dirname(sceneFilePath), filePath});

            Model *model = new Model;
            int    rc = model->Load(filePath);
            if (rc < 0) return rc;
            _models[instance.file] = model;
        }
    }

    return rc;
}

int ModelRenderer::Scene::handleInstanceNode(XmlNode *node, ModelInstance *instance)
{
    string name = node->Tag();
    if (name == TAG_INSTANCE) {
        MyBase::SetErrMsg("Invalid instance \"%s\"", node->Tag().c_str());
        return -1;
    }

    instance->name = name;
    if (node->Attrs().find("file") != node->Attrs().end()) instance->file = node->Attrs()["file"];

    if (node->HasChild("translate"))
        if (handleVectorNode(node->GetChild("translate"), &instance->translate) < 0) return -1;
    if (node->HasChild("rotate"))
        if (handleVectorNode(node->GetChild("rotate"), &instance->rotate) < 0) return -1;
    if (node->HasChild("scale"))
        if (handleVectorNode(node->GetChild("scale"), &instance->scale) < 0) return -1;
    if (node->HasChild("origin")) {
        if (handleVectorNode(node->GetChild("origin"), &instance->origin) < 0) return -1;
    } else if (doesInstanceExist(instance->name)) {
        instance->origin = getInitInstance(instance->name).origin;
    }

    if (doesInstanceExist(instance->name)) {
        if (!instance->file.empty()) {
            MyBase::SetErrMsg("Instance \"%s\" file specified more than once", node->Tag().c_str());
            return -1;
        } else {
            instance->file = getInitInstance(instance->name).file;
        }
    } else {
        if (instance->file.empty()) {
            MyBase::SetErrMsg("Instance \"%s\" defined without an associated file", node->Tag().c_str());
            return -1;
        }

        _instances.push_back(*instance);
    }

    return 0;
}

int ModelRenderer::Scene::handleTimeNode(XmlNode *node)
{
    int    rc = 0;
    int    ts;
    string tsString = node->Tag().substr(strlen(TAG_TIME));

    if (tsString.empty() || parseIntString(tsString, &ts)) {
        MyBase::SetErrMsg("Invalid time tag \"%s\"", node->Tag().c_str());
        return -1;
    }

    vector<ModelInstance> instances;

    for (int i = 0; i < node->GetNumChildren(); i++) {
        XmlNode *child = node->GetChild(i);

        if (STLUtils::BeginsWith(child->Tag(), TAG_INSTANCE)) {
            ModelInstance instance;
            rc = handleInstanceNode(child, &instance);
            if (rc < 0) break;

            instances.push_back(instance);
        } else {
            MyBase::SetErrMsg("Unknown tag \"%s\"", child->Tag().c_str());
            rc = -1;
        }
        if (rc < 0) {
            MyBase::SetErrMsg("%s ->", node->Tag().c_str());
            break;
        }
    }

    _keyframes[ts] = instances;
    return rc;
}

int ModelRenderer::Scene::handleVectorNode(XmlNode *node, glm::vec3 *v)
{
    int rc = 0;
    rc -= handleFloatAttribute(node, "x", &v->x);
    rc -= handleFloatAttribute(node, "y", &v->y);
    rc -= handleFloatAttribute(node, "z", &v->z);
    return rc;
}

int ModelRenderer::Scene::handleFloatAttribute(XmlNode *node, const std::string &name, float *f)
{
    if (node->Attrs().find(name) == node->Attrs().end()) return 0;

    string valueString = node->Attrs()[name];
    size_t charsRead;
    try {
        *f = std::stof(valueString, &charsRead);
    } catch (invalid_argument) {
        MyBase::SetErrMsg("Invalid float attribute %s=\"%s\"", name.c_str(), valueString.c_str());
        return -1;
    } catch (out_of_range) {
        MyBase::SetErrMsg("Float attribute out of range %s=\"%s\"", name.c_str(), valueString.c_str());
        return -1;
    }

    if (charsRead != valueString.size()) {
        MyBase::SetErrMsg("Invalid float attribute %s=\"%s\"", name.c_str(), valueString.c_str());
        return -1;
    }

    return 0;
}

int ModelRenderer::Scene::parseIntString(const std::string &str, int *i) const
{
    size_t charsRead;
    try {
        *i = std::stof(str, &charsRead);
    } catch (invalid_argument) {
        MyBase::SetErrMsg("Invalid integer \"%s\"", str.c_str());
        return -1;
    } catch (out_of_range) {
        MyBase::SetErrMsg("Integer out of range \"%s\"", str.c_str());
        return -1;
    }

    if (charsRead != str.size()) {
        MyBase::SetErrMsg("Invalid integer \"%s\"", str.c_str());
        return -1;
    }

    return 0;
}

ModelRenderer::Scene::ModelInstance ModelRenderer::Scene::getInitInstance(const std::string &name) const
{
    assert(doesInstanceExist(name));

    for (const ModelInstance &inst : _instances)
        if (inst.name == name) return inst;
    return ModelInstance();
}

bool ModelRenderer::Scene::doesInstanceExist(const std::string &name) const
{
    for (const ModelInstance &instance : _instances)
        if (instance.name == name) return true;
    return false;
}

bool ModelRenderer::Scene::isModelCached(const std::string &file) const
{
    for (const auto &it : _models)
        if (it.first == file) return true;
    return false;
}
