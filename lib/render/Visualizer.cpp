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
//	Description:  Implementation of Visualizer class:
//		It performs the opengl rendering for visualizers
//
#include <vapor/glutil.h>    // Must be included first!!!
#include <limits>
#include <algorithm>
#include <vector>
#include <cmath>
#include <cassert>
#ifdef WIN32
    #include <tiff/tiffio.h>
#else
    #include <tiffio.h>
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

#include <vapor/jpegapi.h>
#include <vapor/common.h>
#include "vapor/GLManager.h"
#include "vapor/LegacyGL.h"

#include "imagewriter.hpp"

using namespace VAPoR;
bool Visualizer::_regionShareFlag = true;

/* note:
 * GL_ENUMS used by depth peeling for attaching the color buffers, currently 16 named points exist
 */
GLenum attach_points[] = {GL_COLOR_ATTACHMENT0,  GL_COLOR_ATTACHMENT1,  GL_COLOR_ATTACHMENT2,  GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4,  GL_COLOR_ATTACHMENT5,
                          GL_COLOR_ATTACHMENT6,  GL_COLOR_ATTACHMENT7,  GL_COLOR_ATTACHMENT8,  GL_COLOR_ATTACHMENT9, GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11,
                          GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15};

Visualizer::Visualizer(const ParamsMgr *pm, const DataStatus *dataStatus, string winName)
{
    MyBase::SetDiagMsg("Visualizer::Visualizer() begin");

    m_paramsMgr = pm;
    m_dataStatus = dataStatus;
    m_winName = winName;
    _glManager = nullptr;
    m_vizFeatures = new AnnotationRenderer(pm, dataStatus, winName);
    m_viewpointDirty = true;

    _imageCaptureEnabled = false;
    _animationCaptureEnabled = false;

    _renderOrder.clear();
    _renderer.clear();

    MyBase::SetDiagMsg("Visualizer::Visualizer() end");
}

/*
  Release allocated resources.
*/

Visualizer::~Visualizer()
{
    for (int i = 0; i < _renderer.size(); i++) {
        delete _renderer[i];
#ifdef VAPOR3_0_0_ALPHA
        TextObject::clearTextObjects(_renderer[i]);
#endif
    }
    _renderOrder.clear();
    _renderer.clear();
#ifdef VAPOR3_0_0_ALPHA
    _manipHolder.clear();
#endif
}

//
//  Set up the OpenGL view port, matrix mode, etc.
//

int Visualizer::resizeGL(int wid, int ht)
{
    // Depth buffers are setup, now we need to setup the color textures
    glBindFramebuffer(GL_FRAMEBUFFER, 0);    // prevent framebuffers from being messed with

    // prevent textures from being messed with
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
    if (printOpenGLError()) return -1;
    return 0;
}

int Visualizer::getCurrentTimestep() const
{
    vector<string> dataSetNames = m_dataStatus->GetDataMgrNames();

    bool   first = true;
    size_t min_ts = 0;
    size_t max_ts = 0;
    for (int i = 0; i < dataSetNames.size(); i++) {
        vector<RenderParams *> rParams;
        m_paramsMgr->GetRenderParams(m_winName, dataSetNames[i], rParams);

        if (rParams.size()) {
            // Use local time of first RenderParams instance on window
            // for current data set. I.e. it is assumed that every
            // RenderParams instance for a data set has same current
            // time step.
            //
            size_t local_ts = rParams[0]->GetCurrentTimestep();
            size_t my_min_ts, my_max_ts;
            m_dataStatus->MapLocalToGlobalTimeRange(dataSetNames[i], local_ts, my_min_ts, my_max_ts);
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

void Visualizer::applyTransforms(int i)
{
    string datasetName = _renderer[i]->GetMyDatasetName();
    string myName = _renderer[i]->GetMyName();
    string myType = _renderer[i]->GetMyType();

    VAPoR::ViewpointParams *vpParams = getActiveViewpointParams();
    vector<double>          scales, rotations, translations, origin;
    Transform *             t = vpParams->GetTransform(datasetName);
    assert(t);
    scales = t->GetScales();
    rotations = t->GetRotations();
    translations = t->GetTranslations();
    origin = t->GetOrigin();

    MatrixManager *mm = _glManager->matrixManager;

    mm->MatrixModeModelView();
    mm->PushMatrix();

    mm->Translate(origin[0], origin[1], origin[2]);
    mm->Scale(scales[0], scales[1], scales[2]);
    mm->Rotate(rotations[0], 1, 0, 0);
    mm->Rotate(rotations[1], 0, 1, 0);
    mm->Rotate(rotations[2], 0, 0, 1);
    mm->Translate(-origin[0], -origin[1], -origin[2]);

    mm->Translate(translations[0], translations[1], translations[2]);
}

int Visualizer::paintEvent(bool fast)
{
    MyBase::SetDiagMsg("Visualizer::paintGL()");
    GL_ERR_BREAK();

    // Do not proceed if there is no DataMgr
    if (!m_dataStatus->GetDataMgrNames().size()) return (0);

    if (!fbSetup()) return 0;

    // Set up the OpenGL environment
    int timeStep = getCurrentTimestep();
    if (timeStep < 0) {
        MyBase::SetErrMsg("Invalid time step");
        return -1;
    }

    if (paintSetup(timeStep)) return -1;
    // make sure to capture whenever the time step or frame index changes (once we implement capture!)

    if (timeStep != _previousTimeStep) { _previousTimeStep = timeStep; }

    // Prepare for Renderers
    // Make the depth buffer writable
    glDepthMask(GL_TRUE);
    // and readable
    glEnable(GL_DEPTH_TEST);
    // Prepare for alpha values:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render the current active manip, if there is one

    // Now we are ready for all the different renderers to proceed.
    // Sort them;  If they are opaque, they go first.  If not opaque, they
    // are sorted back-to-front.  Note: This only works if all the geometry of a renderer is ordered by
    // a simple depth test.
    int rc = 0;
    // Now go through all the active renderers, provided the error has not been set
    for (int i = 0; i < _renderer.size(); i++) {
        // If a renderer is not initialized, or if its bypass flag is set, then don't render.
        // Otherwise push and pop the GL matrix stack, and all attribs
        // Push or reset state
        _glManager->matrixManager->MatrixModeModelView();
        _glManager->matrixManager->PushMatrix();

        if (!_renderer[i]->IsGLInitialized()) {
            int myrc = _renderer[i]->initializeGL(_glManager);
            GL_ERR_BREAK();
            if (myrc < 0) rc = -1;
        }

        if (_renderer[i]->IsGLInitialized()) {
            applyTransforms(i);
            int myrc = _renderer[i]->paintGL(fast);
            GL_ERR_BREAK();
            if (myrc < 0) { rc = -1; }
            _glManager->matrixManager->PopMatrix();
            if (myrc < 0) { rc = -1; }
        }
        _glManager->matrixManager->MatrixModeModelView();
        _glManager->matrixManager->PopMatrix();
        int myrc = printOpenGLErrorMsg(_renderer[i]->GetMyName().c_str());
        if (myrc < 0) { rc = -1; }
    }

    if (m_vizFeatures) {
        // Draw the domain frame and other in-scene features
        m_vizFeatures->InScenePaint(timeStep);
        GL_ERR_BREAK();
    }

    // _glManager->ShowDepthBuffer();

    // Go back to MODELVIEW for any other matrix stuff
    // By default the matrix is expected to be MODELVIEW
    _glManager->matrixManager->MatrixModeModelView();

    // Draw any features that are overlaid on scene

    if (m_vizFeatures) m_vizFeatures->DrawText();
    GL_ERR_BREAK();
    renderColorbars(timeStep);
    GL_ERR_BREAK();

    // Perform final touch-up on the final images, before capturing or displaying them.
    glFlush();

    if (_imageCaptureEnabled) {
        captureImage(_captureImageFile);
    } else if (_animationCaptureEnabled) {
        captureImage(_captureImageFile);
        incrementPath(_captureImageFile);
    }
    GL_ERR_BREAK();
    if (printOpenGLError()) return -1;
    return rc;
}

bool Visualizer::fbSetup()
{
#ifdef VAPOR3_0_0_ALPHA
    // Following is needed in case undo/redo leaves a
    // disabled renderer in the renderer list, so it can be deleted.
    //
    removeDisabledRenderers();
#endif

#ifdef VAPOR3_0_0_ALPHA
    // Get the ModelView matrix from the viewpoint params, if it has changed.  If
    // it is not changed, it will come from the Trackball
    if (vpParams->VPHasChanged(_winNum))
#else
    // if (m_viewpointDirty)
#endif

        // Paint background
        double clr[3];
    getActiveAnnotationParams()->GetBackgroundColor(clr);

    glClearColor(clr[0], clr[1], clr[2], 1.f);
    // Clear out the depth buffer in preparation for rendering
    glClearDepth(1);
    // Make the depth buffer writable
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    return (FrameBufferReady());
}

int Visualizer::paintSetup(int timeStep)
{
    assert(!printOpenGLError());
    ViewpointParams *vpParams = getActiveViewpointParams();

    double m[16];
    vpParams->GetProjectionMatrix(m);
    _glManager->matrixManager->MatrixModeProjection();
    _glManager->matrixManager->LoadMatrixd(m);
    assert(!printOpenGLError());

    vpParams->GetModelViewMatrix(m);
    _glManager->matrixManager->MatrixModeModelView();
    _glManager->matrixManager->LoadMatrixd(m);
    assert(!printOpenGLError());

    // Improve polygon antialiasing
    glEnable(GL_MULTISAMPLE);
    assert(!printOpenGLError());

    // Lights are positioned relative to the view direction, do this before the modelView matrix is set
    if (placeLights()) return -1;

    return 0;
}
//
//  Set up the OpenGL rendering state, and define display list
//

int Visualizer::initializeGL(GLManager *glManager)
{
    _glManager = glManager;
    m_vizFeatures->InitializeGL(glManager);

    // glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    assert(GLManager::CheckError());
    if (GLEW_OK != err) {
        MyBase::SetErrMsg("Error: Unable to initialize GLEW");
        return -1;
    }

    glClearColor(1.f, 0.1f, 0.1f, 1.f);
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    GLenum glErr;
    glErr = glGetError();
    if (glErr != GL_NO_ERROR) {
        MyBase::SetErrMsg("Error: No Usable Graphics Driver.\n%s", "Check that drivers are properly installed.");
        return -1;
    }
    _previousTimeStep = -1;
    _previousFrameNum = -1;
    glEnable(GL_MULTISAMPLE);
    if (printOpenGLError()) return -1;
    // Check to see if we are using MESA:
    if (GetVendor() == MESA) { SetErrMsg("GL Vendor String is MESA.\nGraphics drivers may need to be reinstalled"); }

    SetDiagMsg("OpenGL Capabilities : GLEW_VERSION_2_0 %s", GLEW_VERSION_2_0 ? "ok" : "missing");
    SetDiagMsg("OpenGL Capabilities : GLEW_EXT_framebuffer_object %s", GLEW_EXT_framebuffer_object ? "ok" : "missing");
    SetDiagMsg("OpenGL Capabilities : GLEW_ARB_vertex_buffer_object %s", GLEW_ARB_vertex_buffer_object ? "ok" : "missing");
    SetDiagMsg("OpenGL Capabilities : GLEW_ARB_multitexture %s", GLEW_ARB_multitexture ? "ok" : "missing");
    SetDiagMsg("OpenGL Capabilities : GLEW_ARB_shader_objects %s", GLEW_ARB_shader_objects ? "ok" : "missing");

    // Initialize existing renderers:
    //
    if (printOpenGLError()) return -1;

#ifdef VAPOR3_0_0_ALPHA
    if (setUpViewport(_width, _height) < 0) return -1;
#endif
    return 0;
}

void Visualizer::moveRendererToFront(const Renderer *ren)
{
    int renIndex = -1;
    for (int i = 0; i < _renderer.size(); i++) {
        if (_renderer[i] == ren) {
            renIndex = i;
            break;
        }
    }
    assert(renIndex != -1);

    Renderer *save = _renderer[renIndex];
    int       saveOrder = _renderOrder[renIndex];

    for (int i = renIndex; i < _renderer.size() - 1; i++) {
        _renderer[i] = _renderer[i + 1];
        _renderOrder[i] = _renderOrder[i + 1];
    }
    _renderer[_renderer.size() - 1] = save;
    _renderOrder[_renderer.size() - 1] = saveOrder;
}

/*
 * Insert a renderer to this visualizer
 * Add it after all renderers of lower render order
 */
int Visualizer::insertRenderer(Renderer *ren, int newOrder)
{
    // For the first renderer:
    if (_renderer.size() == 0) {
        _renderer.push_back(ren);
        _renderOrder.push_back(newOrder);
        //		ren->initializeGL();
        return 0;
    }
    // Find a renderer of lower order
    int i;
    for (i = _renderer.size() - 1; i >= 0; i--) {
        if (_renderOrder[i] < newOrder) break;
    }
    // Remember the position in front of where this renderer will go:
    int lastPosn = i;
    int maxPosn = _renderer.size() - 1;
    // Push the last one back, increasing the size of these vectors:
    _renderer.push_back(_renderer[maxPosn]);
    _renderOrder.push_back(_renderOrder[maxPosn]);
    // Now the size is maxPosn+1, so copy everything up one position,
    // Until we get to lastPosn (which we do not copy)
    for (i = maxPosn; i > lastPosn + 1; i--) {
        _renderer[i] = _renderer[i - 1];
        _renderOrder[i] = _renderOrder[i - 1];
    }
    // Finally insert the new renderer at lastPosn+1
    _renderer[lastPosn + 1] = ren;

    _renderOrder[lastPosn + 1] = newOrder;
    //	ren->initializeGL();
    return lastPosn + 1;
}

// Remove all renderers.  This is needed when we load new data into
// an existing session
void Visualizer::removeAllRenderers()
{
    // Prevent new rendering while we do this?

#ifdef VAPOR3_0_0_ALPHA
    for (int i = _renderer.size() - 1; i >= 0; i--) { delete _renderer[i]; }
#endif

    _renderOrder.clear();
    _renderer.clear();
}
/*
 * Remove renderer of specified renderParams
 */
bool Visualizer::RemoveRenderer(Renderer *ren)
{
    int i;

    // get it from the renderer list, and delete it:
    bool found = false;
    for (i = 0; i < _renderer.size(); i++) {
        if (_renderer[i] != ren) continue;
#ifdef VAPOR3_0_0_ALPHA
        delete _renderer[i];
#endif

        _renderer[i] = 0;
        found = true;
        break;
    }
    if (!found) return (false);

    int foundIndex = i;

    // Move renderers up.
    int numRenderers = _renderer.size() - 1;
    for (int j = foundIndex; j < numRenderers; j++) {
        _renderer[j] = _renderer[j + 1];
        _renderOrder[j] = _renderOrder[j + 1];
    }
    _renderer.resize(numRenderers);
    _renderOrder.resize(numRenderers);
    return true;
}

Renderer *Visualizer::getRenderer(string type, string instance) const
{
    for (int i = 0; i < _renderer.size(); i++) {
        Renderer *ren = _renderer[i];
        if (ren->GetMyType() == type && ren->GetMyName() == instance) { return (ren); }
    }
    return (NULL);
}

int Visualizer::placeLights()
{
    if (printOpenGLError()) return -1;
    const ViewpointParams *vpParams = getActiveViewpointParams();
    size_t                 nLights = vpParams->getNumLights();
    if (nLights > 3) nLights = 3;
    LegacyGL *lgl = _glManager->legacy;

    float lightDirs[3][4];
    for (int j = 0; j < nLights; j++) {
        for (int i = 0; i < 4; i++) { lightDirs[j][i] = vpParams->getLightDirection(j, i); }
    }
    if (nLights > 0) {
        printOpenGLError();
        float   specColor[4], ambColor[4];
        float   diffLight[3], specLight[3];
        GLfloat lmodel_ambient[4];
        specColor[0] = specColor[1] = specColor[2] = 0.8f;
        ambColor[0] = ambColor[1] = ambColor[2] = 0.f;
        specColor[3] = ambColor[3] = lmodel_ambient[3] = 1.f;

        // TODO GL
        GL_LEGACY(glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, vpParams->getExponent()));
        lmodel_ambient[0] = lmodel_ambient[1] = lmodel_ambient[2] = vpParams->getAmbientCoeff();
        // All the geometry will get a white specular color:
        GL_LEGACY(glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specColor));
        GL_LEGACY(glLightfv(GL_LIGHT0, GL_POSITION, lightDirs[0]));
        lgl->LightDirectionfv(lightDirs[0]);

        specLight[0] = specLight[1] = specLight[2] = vpParams->getSpecularCoeff(0);

        diffLight[0] = diffLight[1] = diffLight[2] = vpParams->getDiffuseCoeff(0);
        GL_LEGACY(glLightfv(GL_LIGHT0, GL_DIFFUSE, diffLight));
        GL_LEGACY(glLightfv(GL_LIGHT0, GL_SPECULAR, specLight));
        GL_LEGACY(glLightfv(GL_LIGHT0, GL_AMBIENT, ambColor));
        GL_LEGACY(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient));
        // Following has unpleasant effects on flow line lighting
        // glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

        GL_LEGACY(glEnable(GL_LIGHT0));
        if (printOpenGLError()) { return -1; }
        if (nLights > 1) {
            if (printOpenGLError()) return -1;
            GL_LEGACY(glLightfv(GL_LIGHT1, GL_POSITION, lightDirs[1]));
            specLight[0] = specLight[1] = specLight[2] = vpParams->getSpecularCoeff(1);
            diffLight[0] = diffLight[1] = diffLight[2] = vpParams->getDiffuseCoeff(1);
            GL_LEGACY(glLightfv(GL_LIGHT1, GL_DIFFUSE, diffLight));
            GL_LEGACY(glLightfv(GL_LIGHT1, GL_SPECULAR, specLight));
            GL_LEGACY(glLightfv(GL_LIGHT1, GL_AMBIENT, ambColor));
            GL_LEGACY(glEnable(GL_LIGHT1));

        } else {
            GL_LEGACY(glDisable(GL_LIGHT1));
            if (printOpenGLError()) return -1;
        }
        if (nLights > 2) {
            GL_LEGACY(glLightfv(GL_LIGHT2, GL_POSITION, lightDirs[2]); specLight[0] = specLight[1] = specLight[2] = vpParams->getSpecularCoeff(2);
                      diffLight[0] = diffLight[1] = diffLight[2] = vpParams->getDiffuseCoeff(2); glLightfv(GL_LIGHT2, GL_DIFFUSE, diffLight); glLightfv(GL_LIGHT2, GL_SPECULAR, specLight);
                      glLightfv(GL_LIGHT2, GL_AMBIENT, ambColor); glEnable(GL_LIGHT2););
            if (printOpenGLError()) return -1;
        } else {
            GL_LEGACY(glDisable(GL_LIGHT2));
        }
    }
    if (printOpenGLError()) return -1;
    return 0;
}

Visualizer::OGLVendorType Visualizer::GetVendor()
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

void Visualizer::removeDisabledRenderers()
{
    // Repeat until we don't find any renderers to disable:

    while (1) {
        bool retry = false;
        for (int i = 0; i < _renderer.size(); i++) {
            RenderParams *rParams = _renderer[i]->GetActiveParams();
            if (!rParams->IsEnabled()) {
                RemoveRenderer(_renderer[i]);
                retry = true;
                break;
            }
        }
        if (!retry) break;
    }
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
ViewpointParams *Visualizer::getActiveViewpointParams() const { return m_paramsMgr->GetViewpointParams(m_winName); }

RegionParams *Visualizer::getActiveRegionParams() const { return m_paramsMgr->GetRegionParams(m_winName); }

AnnotationParams *Visualizer::getActiveAnnotationParams() const { return m_paramsMgr->GetAnnotationParams(m_winName); }

#ifdef VAPOR3_0_0_ALPHA
void Visualizer::resetTrackball()
{
    if (m_trackBall) delete m_trackBall;
    m_trackBall = new Trackball();
}
#endif

int Visualizer::captureImage(string filename)
{
    ViewpointParams *vpParams = getActiveViewpointParams();

    size_t width, height;
    vpParams->GetWindowSize(width, height);

    // Turn off the single capture flag
    _imageCaptureEnabled = false;
    string suffix = filename.substr(filename.length() - 4, 4);    // it assumes fixed length of all suffix...

    FILE *jpegFile = NULL;
    TIFF *tiffFile = NULL;
    if (suffix == ".tif" || suffix == "tiff") {
        tiffFile = TIFFOpen((const char *)filename.c_str(), "wb");
        if (!tiffFile) {
            SetErrMsg("Image Capture Error: Error opening output Tiff file: %s", (const char *)filename.c_str());
            return -1;
        }
    } else if (suffix == ".jpg" || suffix == "jpeg") {
        jpegFile = fopen((const char *)filename.c_str(), "wb");
        if (!jpegFile) {
            SetErrMsg("Image Capture Error: Error opening output Jpeg file: %s", (const char *)filename.c_str());
            return -1;
        }
    } else    // write png files
    {
        // The Write_PNG() function handles fopen et al. by itself.
        // Here we assume the filename is absolutely valid.
    }
    // Get the image buffer
    unsigned char *buf = new unsigned char[3 * width * height];
    // Use openGL to fill the buffer:
    if (!getPixelData(buf)) {
        SetErrMsg("Image Capture Error; error obtaining GL data");
        delete[] buf;
        return -1;
    }

    // Now call the Jpeg or tiff library to compress and write the file
    //
    if (suffix == ".tif" || suffix == "tiff")    // capture the tiff file, one scanline at a time
    {
        uint32 imagelength = (uint32)width;
        uint32 imagewidth = (uint32)height;
        assert(tiffFile);
        TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, imagelength);
        TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, imagewidth);
        TIFFSetField(tiffFile, TIFFTAG_PLANARCONFIG, 1);
        TIFFSetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, 3);
        TIFFSetField(tiffFile, TIFFTAG_ROWSPERSTRIP, 8);
        TIFFSetField(tiffFile, TIFFTAG_BITSPERSAMPLE, 8);
        TIFFSetField(tiffFile, TIFFTAG_PHOTOMETRIC, 2);
        for (int row = 0; row < imagelength; row++) {
            int rc = TIFFWriteScanline(tiffFile, buf + row * 3 * imagewidth, row);
            if (rc != 1) {
                SetErrMsg("Image Capture Error; Error writing tiff file %s", (const char *)filename.c_str());
                break;
            }
        }
        TIFFClose(tiffFile);
    } else if (suffix == ".jpg" || suffix == "jpeg") {
        int quality = 95;
        int rc = write_JPEG_file(jpegFile, width, height, buf, quality);
        fclose(jpegFile);
        if (rc) {
            SetErrMsg("Image Capture Error; Error writing jpeg file %s", (const char *)filename.c_str());
            delete[] buf;
            return -1;
        }
    } else    // PNG
    {
        int rc = Write_PNG(filename.c_str(), width, height, buf);
        if (rc) {
            SetErrMsg("Image Capture Error; Error writing PNG file %s", (const char *)filename.c_str());
            delete[] buf;
            return -1;
        }
    }

    delete[] buf;
    return 0;
}

// Produce an array based on current contents of the (back) buffer
bool Visualizer::getPixelData(unsigned char *data) const
{
    ViewpointParams *vpParams = getActiveViewpointParams();

    size_t width, height;
    vpParams->GetWindowSize(width, height);

    // Must clear previous errors first.
    while (glGetError() != GL_NO_ERROR)
        ;

    glReadBuffer(GL_BACK);
    glDisable(GL_SCISSOR_TEST);

    // Calling pack alignment ensures that we can grab the any size window
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    if (glGetError() != GL_NO_ERROR) return false;
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

void Visualizer::renderColorbars(int timeStep)
{
    MatrixManager *mm = _glManager->matrixManager;
    mm->MatrixModeModelView();
    mm->PushMatrix();
    mm->LoadIdentity();
    mm->MatrixModeProjection();
    mm->PushMatrix();
    mm->LoadIdentity();
    for (int i = 0; i < _renderer.size(); i++) {
        // If a renderer is not initialized, or if its bypass flag is set, then don't render.
        // Otherwise push and pop the GL matrix stack, and all attribs
        _renderer[i]->renderColorbar();
    }
    mm->MatrixModeProjection();
    mm->PopMatrix();
    mm->MatrixModeModelView();
    mm->PopMatrix();
}

void Visualizer::incrementPath(string &s)
{
    // truncate the last 4 characters (remove .tif or .jpg)
    string s1 = s.substr(0, s.length() - 4);
    string s_end = s.substr(s.length() - 4);
    if (s_end == "jpeg") {
        s1 = s.substr(0, s.length() - 5);
        s_end = s.substr(s.length() - 5);
    }
    // Find digits (before .tif or .jpg)
    size_t lastpos = s1.find_last_not_of("0123456789");
    assert(lastpos < s1.length());
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
