//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Visualizer.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		September, 2013
//
#include <vapor/glutil.h>    // Must be included first!!!
#include <limits>
#include <algorithm>
#include <vector>
#include <cmath>
#include "vapor/VAssert.h"
#ifdef WIN32
    #include <tiff/tiffio.h>
#else
    #include <tiffio.h>
    #include <xtiffio.h>
#endif

#ifdef WIN32
    #pragma warning(disable : 4996)
#endif

#include <vapor/RenderParams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/regionparams.h>
#include <vapor/Renderer.h>
#include <vapor/DataStatus.h>
#include <vapor/Visualizer.h>
#include <vapor/FileUtils.h>

#include <vapor/common.h>
#include "vapor/GLManager.h"
#include "vapor/LegacyGL.h"
#include <vapor/ShaderManager.h>

#include "vapor/ImageWriter.h"
#include "vapor/GeoTIFWriter.h"

using namespace VAPoR;

Visualizer::Visualizer(const ParamsMgr *pm, const DataStatus *dataStatus, string winName)
{
    MyBase::SetDiagMsg("Visualizer::Visualizer() begin");

    _paramsMgr = pm;
    _dataStatus = dataStatus;
    _winName = winName;
    _glManager = nullptr;
    _vizFeatures = new AnnotationRenderer(_paramsMgr, _dataStatus, _winName);
    _insideGLContext = false;
    _imageCaptureEnabled = false;
    _animationCaptureEnabled = false;

    _renderers.clear();
    _renderersToDestroy.clear();

    MyBase::SetDiagMsg("Visualizer::Visualizer() end");
}

Visualizer::~Visualizer()
{
#ifdef VAPOR_3_1_0
    // Can't call renderer destructors because these free OpenGL resources that
    // may require the OpenGL context to be current :-(
    //
    for (int i = 0; i < _renderers.size(); i++) delete _renderers[i];
    _renderers.clear();
#endif

    if (_vizFeatures) delete _vizFeatures;

    if (_screenQuadVAO) glDeleteVertexArrays(1, &_screenQuadVAO);
    if (_screenQuadVBO) glDeleteBuffers(1, &_screenQuadVBO);
}

int Visualizer::resizeGL(int wid, int ht) { return 0; }

int Visualizer::_getCurrentTimestep() const
{
    vector<string> dataSetNames = _dataStatus->GetDataMgrNames();

    bool   first = true;
    size_t min_ts = 0;
    size_t max_ts = 0;
    for (int i = 0; i < dataSetNames.size(); i++) {
        vector<RenderParams *> rParams;
        _paramsMgr->GetRenderParams(_winName, dataSetNames[i], rParams);

        if (rParams.size()) {
            // Use local time of first RenderParams instance on window
            // for current data set. I.e. it is assumed that every
            // RenderParams instance for a data set has same current
            // time step.
            //
            size_t local_ts = rParams[0]->GetCurrentTimestep();
            size_t my_min_ts, my_max_ts;
            _dataStatus->MapLocalToGlobalTimeRange(dataSetNames[i], local_ts, my_min_ts, my_max_ts);
            if (first) {
                min_ts = my_min_ts;
                max_ts = my_max_ts;
                first = false;
            } else {
                if (my_min_ts > min_ts) min_ts = my_min_ts;
                if (my_max_ts < max_ts) max_ts = my_max_ts;
            }
        }
    }
    if (min_ts > max_ts) return (-1);

    return (min_ts);
}

void Visualizer::_applyDatasetTransformsForRenderer(Renderer *r)
{
    string datasetName = r->GetMyDatasetName();
    string myName = r->GetMyName();
    string myType = r->GetMyType();

    VAPoR::ViewpointParams *vpParams = getActiveViewpointParams();
    vector<double>          scales, rotations, translations, origin;
    Transform *             t = vpParams->GetTransform(datasetName);
    VAssert(t);
    scales = t->GetScales();
    rotations = t->GetRotations();
    translations = t->GetTranslations();
    origin = t->GetOrigin();

    MatrixManager *mm = _glManager->matrixManager;

    mm->Translate(origin[0], origin[1], origin[2]);
    mm->Rotate(glm::radians(rotations[0]), 1, 0, 0);
    mm->Rotate(glm::radians(rotations[1]), 0, 1, 0);
    mm->Rotate(glm::radians(rotations[2]), 0, 0, 1);
    mm->Scale(scales[0], scales[1], scales[2]);
    mm->Translate(-origin[0], -origin[1], -origin[2]);

    mm->Translate(translations[0], translations[1], translations[2]);
}

int Visualizer::paintEvent(bool fast)
{
    _insideGLContext = true;
    MyBase::SetDiagMsg("Visualizer::paintGL()");
    GL_ERR_BREAK();

    MatrixManager *mm = _glManager->matrixManager;

    // Do not proceed if there is no DataMgr
    if (!_paramsMgr->GetDataMgrNames().size()) return (0);

    // Do not proceed with invalid viewport
    // This can occur sometimes on Qt startup
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] <= 0) return 0;
    if (viewport[3] <= 0) return 0;

    _clearActiveFramebuffer(0.3, 0.3, 0.3);

    int fbWidth = viewport[2];
    int fbHeight = viewport[3];

    ViewpointParams *vp = getActiveViewpointParams();
    if (vp->GetValueLong(ViewpointParams::UseCustomFramebufferTag, 0)) {
        fbWidth = vp->GetValueLong(ViewpointParams::CustomFramebufferWidthTag, 0);
        fbHeight = vp->GetValueLong(ViewpointParams::CustomFramebufferHeightTag, 0);
    }
    _framebuffer.SetSize(fbWidth, fbHeight);
    _framebuffer.MakeRenderTarget();

    double clr[3];
    getActiveAnnotationParams()->GetBackgroundColor(clr);
    _clearActiveFramebuffer(clr[0], clr[1], clr[2]);

    _loadMatricesFromViewpointParams();
    if (_configureLighting()) return -1;

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw the domain frame and other in-scene features
    _vizFeatures->InScenePaint(_getCurrentTimestep());
    GL_ERR_BREAK();

    _deleteFlaggedRenderers();
    if (_initializeNewRenderers() < 0) return -1;

    int rc = 0;
    for (int i = 0; i < _renderers.size(); i++) {
        _glManager->matrixManager->MatrixModeModelView();
        _glManager->matrixManager->PushMatrix();

        if (_renderers[i]->IsGLInitialized()) {
            _applyDatasetTransformsForRenderer(_renderers[i]);

            //            void *t = _glManager->BeginTimer();
            int myrc = _renderers[i]->paintGL(fast);
            //            printf("%s: %f\n", _renderers[i]->GetMyName().c_str(), _glManager->EndTimer(t));
            GL_ERR_BREAK();
            if (myrc < 0) rc = -1;
        }
        mm->MatrixModeModelView();
        mm->PopMatrix();
        int myrc = CheckGLErrorMsg(_renderers[i]->GetMyName().c_str());
        if (myrc < 0) rc = -1;
    }

    _vizFeatures->DrawText();
    GL_ERR_BREAK();
    _renderColorbars(_getCurrentTimestep());
    GL_ERR_BREAK();
    _vizFeatures->DrawAxisArrows();
    GL_ERR_BREAK();

    //    _glManager->ShowDepthBuffer();

    glFlush();

    int captureImageSuccess = 0;
    if (_imageCaptureEnabled) {
        captureImageSuccess = _captureImage(_captureImageFile);
    } else if (_animationCaptureEnabled) {
        captureImageSuccess = _captureImage(_captureImageFile);
        _incrementPath(_captureImageFile);
    }
    if (captureImageSuccess < 0) {
        SetErrMsg("Failed to save image");
        return -1;
    }

    GL_ERR_BREAK();
    if (CheckGLError()) return -1;

    _framebuffer.UnBind();
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glBindVertexArray(_screenQuadVAO);
    SmartShaderProgram shader = _glManager->shaderManager->GetSmartShader("Framebuffer");
    if (!shader.IsValid()) return -1;
    shader->SetSampler("colorBuffer", *_framebuffer.GetColorTexture());
    shader->SetSampler("depthBuffer", *_framebuffer.GetDepthTexture());

    glDepthFunc(GL_LEQUAL);
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

    _insideGLContext = false;
    return rc;
}

void Visualizer::_loadMatricesFromViewpointParams()
{
    ViewpointParams *const vpParams = getActiveViewpointParams();
    MatrixManager *const   mm = _glManager->matrixManager;

    double m[16];
    vpParams->GetProjectionMatrix(m);
    mm->MatrixModeProjection();
    mm->LoadMatrixd(m);

    vpParams->GetModelViewMatrix(m);
    mm->MatrixModeModelView();
    mm->LoadMatrixd(m);
}

int Visualizer::InitializeGL(GLManager *glManager)
{
    if (!glManager->IsCurrentOpenGLVersionSupported()) return -1;

    _glManager = glManager;
    _vizFeatures->InitializeGL(glManager);

    // glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    VAssert(GLManager::CheckError());
    if (GLEW_OK != err) {
        MyBase::SetErrMsg("Error: Unable to initialize GLEW");
        return -1;
    }

    if (GetVendor() == MESA) { SetErrMsg("GL Vendor String is MESA.\nGraphics drivers may need to be reinstalled"); }

    _framebuffer.EnableDepthBuffer();
    _framebuffer.Generate();

    static float data[] = {-1, -1, 0, 0, 1, -1, 1, 0, -1, 1, 0, 1,

                           -1, 1,  0, 1, 1, -1, 1, 0, 1,  1, 1, 1};
    glGenVertexArrays(1, &_screenQuadVAO);
    glGenBuffers(1, &_screenQuadVBO);
    glBindVertexArray(_screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    return 0;
}

// Move to back of rendering list
void Visualizer::MoveRendererToFront(string renderType, string renderName)
{
    Renderer *ren = _getRenderer(renderType, renderName);
    if (!ren) return;

    auto it = std::find(_renderers.begin(), _renderers.end(), ren);
    VAssert(it != _renderers.end());
    _renderers.erase(it);
    _renderers.push_back(ren);
}

void Visualizer::MoveRenderersOfTypeToFront(const std::string &type)
{
    Renderer *firstRendererMoved = nullptr;
    auto      rendererPointersCopy = _renderers;
    for (auto it = rendererPointersCopy.rbegin(); it != rendererPointersCopy.rend(); ++it) {
        if (*it == firstRendererMoved) break;
        if ((*it)->GetMyType() == type) {
            MoveRendererToFront((*it)->GetMyType(), (*it)->GetMyName());
            if (firstRendererMoved == nullptr) firstRendererMoved = *it;
        }
    }
}

int Visualizer::CreateRenderer(string dataSetName, string renderType, string renderName)
{
    if (HasRenderer(renderType, renderName)) return (0);

    Renderer *ren = RendererFactory::Instance()->CreateInstance(_paramsMgr, _winName, dataSetName, renderType, renderName, _dataStatus->GetDataMgr(dataSetName));

    if (!ren) {
        SetErrMsg("Invalid renderer of type \"%s\"", renderType.c_str());
        return (-1);
    }

    _renderers.push_back(ren);
    return (0);
}

void Visualizer::DestroyRenderer(string renderType, string renderName, bool haveOpenGLContext)
{
    Renderer *ren = _getRenderer(renderType, renderName);
    if (!ren) return;

    _renderers.erase(std::find(_renderers.begin(), _renderers.end(), ren));

    // If we have an active OpenGL context destroy the renderer immediately. Else
    // queue for later destruction
    //
    if (haveOpenGLContext) {
        delete ren;
        return;
    }

    // Don't add to queue if already there
    //
    for (auto it = _renderersToDestroy.begin(); it != _renderersToDestroy.end(); ++it) {
        if (*it == ren) return;
    }

    _renderersToDestroy.push_back(ren);
}

void Visualizer::DestroyAllRenderers(bool hasOpenGLContext)
{
    vector<Renderer *> renderersCopy = _renderers;

    for (auto it = renderersCopy.begin(); it != renderersCopy.end(); ++it) {
        Renderer *ren = *it;
        DestroyRenderer(ren->GetMyType(), ren->GetMyName(), hasOpenGLContext);
    }
}

bool Visualizer::HasRenderer(string renderType, string renderName) const { return (_getRenderer(renderType, renderName) != nullptr); }

void Visualizer::ClearRenderCache()
{
    for (int i = 0; i < _renderers.size(); i++) { _renderers[i]->ClearCache(); }
}

Renderer *Visualizer::_getRenderer(string type, string instance) const
{
    for (auto it = _renderers.begin(); it != _renderers.end(); ++it) {
        Renderer *ren = *it;
        if (ren->GetMyType() == type && ren->GetMyName() == instance) { return (ren); }
    }
    return (NULL);
}

int Visualizer::_configureLighting()
{
    const ViewpointParams *vpParams = getActiveViewpointParams();
    size_t                 nLights = vpParams->getNumLights();
    VAssert(nLights <= 1);
    LegacyGL *lgl = _glManager->legacy;

    float lightDir[4];
    for (int i = 0; i < 4; i++) { lightDir[i] = vpParams->getLightDirection(0, i); }

    if (nLights > 0) {
        // TODO GL
        // GL_SHININESS = vpParams->getExponent())
        // vpParams->getSpecularCoeff(0)
        // vpParams->getDiffuseCoeff(0)
        // vpParams->getAmbientCoeff()
        // All the geometry will get a white specular color:

        lgl->LightDirectionfv(lightDir);
    }
    if (CheckGLError()) return -1;
    return 0;
}

Visualizer::GLVendorType Visualizer::GetVendor()
{
    string ven_str((const char *)glGetString(GL_VENDOR));
    string ren_str((const char *)glGetString(GL_RENDERER));

    for (int i = 0; i < ven_str.size(); i++) {
        if (isupper(ven_str[i])) ven_str[i] = tolower(ven_str[i]);
    }
    for (int i = 0; i < ren_str.size(); i++) {
        if (isupper(ren_str[i])) ren_str[i] = tolower(ren_str[i]);
    }

    if ((ven_str.find("mesa") != string::npos) || (ren_str.find("mesa") != string::npos)) {
        return (MESA);
    } else if ((ven_str.find("nvidia") != string::npos) || (ren_str.find("nvidia") != string::npos)) {
        return (NVIDIA);
    } else if ((ven_str.find("ati") != string::npos) || (ren_str.find("ati") != string::npos)) {
        return (ATI);
    } else if ((ven_str.find("intel") != string::npos) || (ren_str.find("intel") != string::npos)) {
        return (INTEL);
    }
    return (UNKNOWN);
}

double Visualizer::getPixelSize() const
{
#ifdef VAPOR3_0_0_ALPHA
    double temp[3];

    // Window height is subtended by viewing angle (45 degrees),
    // at viewer distance (dist from camera to view center)
    const AnnotationParams *vfParams = getActiveAnnotationParams();
    const ViewpointParams * vpParams = getActiveViewpointParams();

    size_t width, height;
    vpParams->GetWindowSize(width, height);

    double center[3], pos[3];
    vpParams->GetRotationCenter(center);
    vpParams->GetCameraPos(pos);

    vsub(center, pos, temp);

    // Apply stretch factor:

    vector<double> stretch = vpParams->GetStretchFactors();
    for (int i = 0; i < 3; i++) temp[i] = stretch[i] * temp[i];
    float distToScene = vlength(temp);
    // tan(45 deg *0.5) is ratio between half-height and dist to scene
    double halfHeight = tan(M_PI * 0.125) * distToScene;
    return (2.f * halfHeight / (double)height);

#endif
    return (0.0);
}

ViewpointParams *Visualizer::getActiveViewpointParams() const { return _paramsMgr->GetViewpointParams(_winName); }

RegionParams *Visualizer::getActiveRegionParams() const { return _paramsMgr->GetRegionParams(_winName); }

AnnotationParams *Visualizer::getActiveAnnotationParams() const { return _paramsMgr->GetAnnotationParams(_winName); }

int Visualizer::_captureImage(std::string path)
{
    // Turn off the single capture flag
    _imageCaptureEnabled = false;

    if (FileUtils::Extension(path) == "") path += ".png";

    ViewpointParams *vpParams = getActiveViewpointParams();
    int              width, height;
    //	vpParams->GetWindowSize(width, height);
    _framebuffer.GetSize(&width, &height);

    bool geoTiffOutput = vpParams->GetProjectionType() == ViewpointParams::MapOrthographic && (FileUtils::Extension(path) == "tif" || FileUtils::Extension(path) == "tiff");

    ImageWriter *  writer = nullptr;
    unsigned char *framebuffer = nullptr;
    int            writeReturn = -1;

    framebuffer = new unsigned char[3 * width * height];
    if (!_getPixelData(framebuffer))
        ;    // goto captureImageEnd;

    if (geoTiffOutput)
        writer = new GeoTIFWriter(path);
    else
        writer = ImageWriter::CreateImageWriterForFile(path);
    if (writer == nullptr) goto captureImageEnd;

    if (geoTiffOutput) {
        VAssert(_dataStatus->GetDataMgrNames().size());
        string projString = _dataStatus->GetDataMgr(_dataStatus->GetDataMgrNames()[0])->GetMapProjection();

        vector<double> dataMinExtents, dataMaxExtents;
        _dataStatus->GetActiveExtents(_paramsMgr, _winName, _getCurrentTimestep(), dataMinExtents, dataMaxExtents);

        double m[16];
        vpParams->GetModelViewMatrix(m);
        double posvec[3], upvec[3], dirvec[3];
        vpParams->ReconstructCamera(m, posvec, upvec, dirvec);

        float s = vpParams->GetOrthoProjectionSize();
        float x = posvec[0];
        float y = posvec[1];
        float aspect = width / (float)height;

        float pixelScale[2] = {s * aspect * 2 / (float)width, s * 2 / (float)height};

        // Crop to data extents

        double cameraMinExtents[2] = {x - s * aspect, y - s};
        double cameraMaxExtents[2] = {x + s * aspect, y + s};

        int    cropMin[2] = {0, 0};
        double newCameraMinExtents[2] = {cameraMinExtents[0], cameraMinExtents[1]};
        for (int i = 0; i < 2; i++) {
            if (cameraMinExtents[i] < dataMinExtents[i]) {
                newCameraMinExtents[i] = dataMinExtents[i];
                cropMin[i] = (dataMinExtents[i] - cameraMinExtents[i]) / pixelScale[i];
            }
        }

        int    cropMax[2] = {(int)width, (int)height};
        double newCameraMaxExtents[2] = {cameraMaxExtents[0], cameraMaxExtents[1]};
        for (int i = 0; i < 2; i++) {
            if (cameraMaxExtents[i] > dataMaxExtents[i]) {
                newCameraMaxExtents[i] = dataMaxExtents[i];
                cropMax[i] = cropMax[i] - (cameraMaxExtents[i] - dataMaxExtents[i]) / pixelScale[i];
            }
        }

        int croppedWidth = cropMax[0] - cropMin[0];
        int croppedHeight = cropMax[1] - cropMin[1];

        if (croppedWidth <= 0 || croppedHeight <= 0) {
            MyBase::SetErrMsg("Dataset not visible");
            writeReturn = -1;
            goto captureImageEnd;
        }

        // flip Y
        int temp = cropMin[1];
        cropMin[1] = height - cropMax[1];
        cropMax[1] = height - temp;

        unsigned char *croppedFB = new unsigned char[3 * croppedWidth * croppedHeight];
        for (int y = 0; y < croppedHeight; y++) memcpy(&croppedFB[3 * y * croppedWidth], &framebuffer[3 * ((y + cropMin[1]) * width + cropMin[0])], 3 * croppedWidth);

        delete[] framebuffer;
        framebuffer = croppedFB;

        framebuffer = croppedFB;
        s *= croppedHeight / (float)height;

        x = (newCameraMaxExtents[0] - newCameraMinExtents[0]) / 2 + newCameraMinExtents[0];
        y = (newCameraMaxExtents[1] - newCameraMinExtents[1]) / 2 + newCameraMinExtents[1];

        width = croppedWidth;
        height = croppedHeight;
        aspect = width / (float)height;

        GeoTIFWriter *geo = (GeoTIFWriter *)writer;
        geo->SetTiePoint(x, y, width / 2.f, height / 2.f);
        geo->SetPixelScale(s * aspect * 2 / (float)width, s * 2 / (float)height);
        if (geo->ConfigureFromProj4(projString) < 0) {
            writeReturn = -1;
            goto captureImageEnd;
        }
    }

    writeReturn = writer->Write(framebuffer, width, height);

captureImageEnd:
    if (writer) delete writer;
    if (framebuffer) delete[] framebuffer;

    return writeReturn;
}

bool Visualizer::_getPixelData(unsigned char *data) const
{
    int width, height;
    //	vpParams->GetWindowSize(width, height);
    _framebuffer.GetSize(&width, &height);

    // Must clear previous errors first.
    while (glGetError() != GL_NO_ERROR)
        ;

    //	glReadBuffer(GL_BACK);
    //	glDisable(GL_SCISSOR_TEST);

    // Calling pack alignment ensures that we can grab the any size window
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    if (glGetError() != GL_NO_ERROR) {
        SetErrMsg("Error obtaining GL framebuffer data");
        return false;
    }
    // Unfortunately gl reads these in the reverse order that jpeg expects, so
    // Now we need to swap top and bottom!
    unsigned char val;
    for (int j = 0; j < height / 2; j++) {
        for (int i = 0; i < width * 3; i++) {
            val = data[i + width * 3 * j];
            data[i + width * 3 * j] = data[i + width * 3 * (height - j - 1)];
            data[i + width * 3 * (height - j - 1)] = val;
        }
    }

    return true;
}

void Visualizer::_deleteFlaggedRenderers()
{
    VAssert(_insideGLContext);

    for (auto it = _renderersToDestroy.begin(); it != _renderersToDestroy.end(); ++it) {
        Renderer *ren = *it;
        if (ren) delete ren;
    }
    _renderersToDestroy.clear();
}

int Visualizer::_initializeNewRenderers()
{
    VAssert(_insideGLContext);
    for (Renderer *r : _renderers) {
        if (!r->IsGLInitialized() && r->initializeGL(_glManager) < 0) {
            MyBase::SetErrMsg("Failed to initialize renderer %s", r->GetInstanceName().c_str());
            return -1;
        }
        GL_ERR_BREAK();
    }
    return 0;
}

void Visualizer::_clearActiveFramebuffer(float r, float g, float b) const
{
    VAssert(_insideGLContext);

    glDepthMask(GL_TRUE);
    glClearColor(r, g, b, 1.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void Visualizer::_renderColorbars(int timeStep)
{
    MatrixManager *mm = _glManager->matrixManager;
    mm->MatrixModeModelView();
    mm->PushMatrix();
    mm->LoadIdentity();
    mm->MatrixModeProjection();
    mm->PushMatrix();
    mm->LoadIdentity();
    for (int i = 0; i < _renderers.size(); i++) {
        // If a renderer is not initialized, or if its bypass flag is set, then don't render.
        // Otherwise push and pop the GL matrix stack, and all attribs
        _renderers[i]->renderColorbar();
    }
    mm->MatrixModeProjection();
    mm->PopMatrix();
    mm->MatrixModeModelView();
    mm->PopMatrix();
}

void Visualizer::_incrementPath(string &s)
{
    // truncate the last 4 characters (remove .tif or .jpg)
    string s1 = s.substr(0, s.length() - 4);
    string s_end = s.substr(s.length() - 4);
    if (s_end == "jpeg" || s_end == "tiff") {
        s1 = s.substr(0, s.length() - 5);
        s_end = s.substr(s.length() - 5);
    }
    // Find digits (before .tif or .jpg)
    size_t lastpos = s1.find_last_not_of("0123456789");
    VAssert(lastpos < s1.length());
    string s2 = s1.substr(lastpos + 1);
    int    val = stol(s2);
    // Convert val+1 to a string, with leading zeroes, of same length as s2.
    // Except, if val+1 has more digits than s2, increase it.
    int numDigits = 1 + (int)log10((float)(val + 1));
    int len = s2.length();
    if (len < numDigits) len = numDigits;
    if (len > 9) len = 9;
    char format[5] = {"%04d"};
    char result[100];
    char c = '0' + len;
    format[2] = c;
    sprintf(result, format, val + 1);
    string sval = string(result);
    string newbody = s.substr(0, lastpos + 1);
    s = newbody + sval + s_end;
}
