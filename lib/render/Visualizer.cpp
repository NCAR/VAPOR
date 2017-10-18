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
#include <vapor/ShaderMgr.h>

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
    m_shaderMgr = NULL;
    m_vizFeatures = new VizFeatureRenderer(pm, dataStatus, winName);
    m_viewpointDirty = true;

    _imageCaptureEnabled = false;
    _animationCaptureEnabled = false;

    _renderOrder.clear();
    _renderer.clear();

#ifdef DEAD
    // Create Manips for every mode except 0
    _manipHolder.push_back(0);
    for (int i = 1; i < MouseModeParams::getNumMouseModes(); i++) {
        int manipType = MouseModeParams::getModeManipType(i);
        if (manipType == 1 || manipType == 2) {
            TranslateStretchManip *manip = new TranslateStretchManip(this, 0);
            _manipHolder.push_back(manip);
        } else if (manipType == 3) {
            TranslateRotateManip *manip = new TranslateRotateManip(this, 0);
            _manipHolder.push_back(manip);
        } else
            assert(0);
    }
#endif

    MyBase::SetDiagMsg("Visualizer::Visualizer() end");
}

/*
  Release allocated resources.
*/

Visualizer::~Visualizer()
{
    for (int i = 0; i < _renderer.size(); i++) {
        delete _renderer[i];
#ifdef DEAD
        TextObject::clearTextObjects(_renderer[i]);
#endif
    }
    _renderOrder.clear();
    _renderer.clear();
#ifdef DEAD
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
    vector<double>          scales, rotations, translations;
    scales = vpParams->GetScales(datasetName);
    //	rotations = vpParams->GetRotations(datasetName);
    translations = vpParams->GetTranslations(datasetName);

    //  Box was returning extents of 0 and 1????
    //
    //	RegionParams* rParams = getActiveRegionParams();
    //	Box* box = rParams->GetBox();
    //	box->GetExtents(minExts, maxExts);

#ifdef DEAD
    vector<double> minExts, maxExts;
    DataMgr *      dMgr = m_dataStatus->GetDataMgr(datasetName);
    dMgr->GetVariableExtents(0, "U", 3, minExts, maxExts);

    float xCenter = (minExts[0] + maxExts[0]) / 2.f;
    float yCenter = (minExts[1] + maxExts[1]) / 2.f;
    float zCenter = (minExts[2] + maxExts[2]) / 2.f;
#endif

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

#ifdef DEAD
    glTranslatef(xCenter, yCenter, zCenter);
    glRotatef(rotations[0], 1, 0, 0);
    glRotatef(rotations[1], 0, 1, 0);
    glRotatef(rotations[2], 0, 0, 1);
#endif

    glScalef(scales[0], scales[1], scales[2]);
    glTranslatef(translations[0], translations[1], translations[2]);

#ifdef DEAD
    glTranslatef(-xCenter, -yCenter, -zCenter);
#endif
}

int Visualizer::paintEvent()
{
    MyBase::SetDiagMsg("Visualizer::paintGL()");

    // Do not proceed if there is no DataMgr
    if (!m_dataStatus->GetDataMgrNames().size()) return (0);

    if (!fbSetup()) return (0);

    // Set up the OpenGL environment
    int timeStep = getCurrentTimestep();
    if (timeStep < 0) {
        MyBase::SetErrMsg("Invalid time step");
        return -1;
    }

    if (paintSetup(timeStep)) return -1;
    // make sure to capture whenever the time step or frame index changes (once we implement capture!)

    if (timeStep != _previousTimeStep) { _previousTimeStep = timeStep; }

    if (m_vizFeatures) {
        // Draw the domain frame and other in-scene features
        //
        m_vizFeatures->InScenePaint(timeStep);
    }

    // Prepare for Renderers
    // Make the depth buffer writable
    glDepthMask(GL_TRUE);
    // and readable
    glEnable(GL_DEPTH_TEST);
    // Prepare for alpha values:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render the current active manip, if there is one
#ifdef DEAD
    renderManip();
#endif

#ifdef DEAD
    // Render all of the current text objects
    TextObject::renderAllText(timeStep, this);
#endif
    // Now we are ready for all the different renderers to proceed.
    // Sort them;  If they are opaque, they go first.  If not opaque, they
    // are sorted back-to-front.  Note: This only works if all the geometry of a renderer is ordered by
    // a simple depth test.

    int rc = 0;
    // Now go through all the active renderers, provided the error has not been set
    for (int i = 0; i < _renderer.size(); i++) {
        // If a renderer is not initialized, or if its bypass flag is set, then don't render.
        // Otherwise push and pop the GL matrix stack, and all attribs
#ifdef DEAD
        if (_renderer[i]->isInitialized() && !(_renderer[i]->doAlwaysBypass(timeStep))) {
#endif
            {
                // Push or reset state
                glMatrixMode(GL_TEXTURE);
                glPushMatrix();
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glPushAttrib(GL_ALL_ATTRIB_BITS);

                if (!_renderer[i]->IsGLInitialized()) {
                    int myrc = _renderer[i]->initializeGL(m_shaderMgr);
                    if (myrc < 0) rc = -1;
                }

                if (_renderer[i]->IsGLInitialized()) {
                    applyTransforms(i);
                    int myrc = _renderer[i]->paintGL();
                    glPopMatrix();
                    if (myrc < 0) rc = -1;
                }
#ifdef DEAD
                if (rc) { _renderer[i]->setBypass(timeStep); }
#endif
                glPopAttrib();
                glMatrixMode(GL_MODELVIEW);
                glPopMatrix();
                glMatrixMode(GL_TEXTURE);
                glPopMatrix();
                int myrc = printOpenGLErrorMsg(_renderer[i]->GetMyName().c_str());
                if (myrc < 0) rc = -1;
            }
        }

        // Go back to MODELVIEW for any other matrix stuff
        // By default the matrix is expected to be MODELVIEW
        glMatrixMode(GL_MODELVIEW);

        // Draw any features that are overlaid on scene

        if (m_vizFeatures) m_vizFeatures->DrawText();
        renderColorbars(timeStep);
#ifdef DEAD
        if (m_vizFeatures) m_vizFeatures->OverlayPaint(timeStep);
#endif

        // Perform final touch-up on the final images, before capturing or displaying them.
        glFlush();

        if (_imageCaptureEnabled)
            captureImage(_captureImageFile);
        else if (_animationCaptureEnabled) {
            captureImage(_captureImageFile);
            incrementPath(_captureImageFile);
        }
        glDisable(GL_NORMALIZE);
        if (printOpenGLError()) return -1;
        return rc;
    }

    bool Visualizer::fbSetup()
    {
#ifdef DEAD
        // Following is needed in case undo/redo leaves a
        // disabled renderer in the renderer list, so it can be deleted.
        //
        removeDisabledRenderers();
#endif

#ifdef DEAD
        // Get the ModelView matrix from the viewpoint params, if it has changed.  If
        // it is not changed, it will come from the Trackball
        if (vpParams->VPHasChanged(_winNum))
#else
    // if (m_viewpointDirty)
#endif

            // Paint background
            double clr[3];
        getActiveVizFeatureParams()->GetBackgroundColor(clr);

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
        ViewpointParams *vpParams = getActiveViewpointParams();

        double m[16];
        vpParams->GetProjectionMatrix(m);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(m);

        vpParams->GetModelViewMatrix(m);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixd(m);

        // Improve polygon antialiasing
        glEnable(GL_MULTISAMPLE);

        // automatically renormalize normals (especially needed for flow rendering)
        glEnable(GL_NORMALIZE);

        // Lights are positioned relative to the view direction, do this before the modelView matrix is set
        if (placeLights()) { return -1; }

#ifdef DEAD
        double center[3];
        m_trackBall->GetCenter(center);
        vector<double> stretch = vpParams->GetStretchFactors();

        glTranslated(center[0], center[1], center[2]);
        glScaled(stretch[0], stretch[1], stretch[2]);
        glTranslated(-center[0], -center[1], -center[2]);
#endif

#ifdef DEAD
        // Save the GL matrix in the viewpoint params, for when the mouse is moving.
        // Don't put this event in the command queue.
        if (_tBallChanged) {
            vpParams->SetSaveState(false);
            saveGLMatrix(timeStep, vpParams);
            vpParams->SetSaveState(true);
        }
        // Reset the flags
        _tBallChanged = false;
#endif

#ifdef DEAD
        vpParams->VPSetChanged(false);
#endif
        return 0;
    }
    //
    //  Set up the OpenGL rendering state, and define display list
    //

    int Visualizer::initializeGL(ShaderMgr * shaderMgr)
    {
        m_shaderMgr = shaderMgr;
        m_vizFeatures->InitializeGL(shaderMgr);

        GLenum err = glewInit();
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

#ifdef DEAD
        if (setUpViewport(_width, _height) < 0) return -1;
#endif
        return 0;
    }

    // projectPointToWin returns true if point is in front of camera
    // resulting screen coords returned in 2nd argument.  Note that
    // OpenGL coords are 0 at bottom of window!
    //
    bool Visualizer::projectPointToWin(double cubeCoords[3], double winCoords[2])
    {
        double   depth;
        GLdouble wCoords[2];
        GLdouble cbCoords[3];
        for (int i = 0; i < 3; i++) cbCoords[i] = (double)cubeCoords[i];

        const ViewpointParams *vpParams = getActiveViewpointParams();
        double                 mvMatrix[16];
        double                 pMatrix[16];
        vpParams->GetModelViewMatrix(mvMatrix);
        vpParams->GetProjectionMatrix(pMatrix);

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        bool success = (0 != gluProject(cbCoords[0], cbCoords[1], cbCoords[2], mvMatrix, pMatrix, viewport, wCoords, (wCoords + 1), (GLdouble *)(&depth)));
        if (!success) return false;
        winCoords[0] = wCoords[0];
        winCoords[1] = wCoords[1];
        return (depth > 0.0);
    }

    // Convert a screen coord to a vector, representing the displacedment
    // from the camera associated with the screen coords.  Note screen coords
    // are OpenGL style.  strHandleMid is in stretched coordinates.
    //
    bool Visualizer::pixelToVector(double winCoords[2], const vector<double> camPosStr, double dirVec[3], double strHandleMid[3])
    {
        const VizFeatureParams *vfParams = getActiveVizFeatureParams();
        const ViewpointParams * vpParams = getActiveViewpointParams();

        GLdouble pt[3];
        // Project handleMid to find its z screen coordinate:
        GLdouble screenx, screeny, screenz;
        double   mvMatrix[16];
        double   pMatrix[16];
        vpParams->GetModelViewMatrix(mvMatrix);
        vpParams->GetProjectionMatrix(pMatrix);

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        gluProject(strHandleMid[0], strHandleMid[1], strHandleMid[2], mvMatrix, pMatrix, viewport, &screenx, &screeny, &screenz);
        // Obtain the coords of a point in view:
        bool success = (0 != gluUnProject((GLdouble)winCoords[0], (GLdouble)winCoords[1], screenz, mvMatrix, pMatrix, viewport, pt, pt + 1, pt + 2));
        if (success) {
            // transform position to world coords not needed, but need to unstretch(?)
            vector<double> stretch = vpParams->GetStretchFactors();

            // Subtract camera coords to get a direction vector:
            vsub(pt, camPosStr, dirVec);
            // unstretch the difference:
            for (int i = 0; i < 3; i++) dirVec[i] = dirVec[i] / stretch[i];
        }
        return success;
    }
    // Test if the screen projection of a 3D quad encloses a point on the screen.
    // The 4 corners of the quad must be specified in counter-clockwise order
    // as viewed from the outside (pickable side) of the quad.
    // Window coords are as in OpenGL (0 at bottom of window)
    //
    bool Visualizer::pointIsOnQuad(double cor1[3], double cor2[3], double cor3[3], double cor4[3], double pickPt[2])
    {
        double winCoord1[2];
        double winCoord2[2];
        double winCoord3[2];
        double winCoord4[2];
        if (!projectPointToWin(cor1, winCoord1)) return false;
        if (!projectPointToWin(cor2, winCoord2)) return false;
        if (pointOnRight(winCoord1, winCoord2, pickPt)) return false;
        if (!projectPointToWin(cor3, winCoord3)) return false;
        if (pointOnRight(winCoord2, winCoord3, pickPt)) return false;
        if (!projectPointToWin(cor4, winCoord4)) return false;
        if (pointOnRight(winCoord3, winCoord4, pickPt)) return false;
        if (pointOnRight(winCoord4, winCoord1, pickPt)) return false;
        return true;
    }
    // Test whether the pickPt is over (and outside) the box (as specified by 8 points)
    int Visualizer::pointIsOnBox(double corners[8][3], double pickPt[2])
    {
        // front (-Z)
        if (pointIsOnQuad(corners[0], corners[1], corners[3], corners[2], pickPt)) return 2;
        // back (+Z)
        if (pointIsOnQuad(corners[4], corners[6], corners[7], corners[5], pickPt)) return 3;
        // right (+X)
        if (pointIsOnQuad(corners[1], corners[5], corners[7], corners[3], pickPt)) return 5;
        // left (-X)
        if (pointIsOnQuad(corners[0], corners[2], corners[6], corners[4], pickPt)) return 0;
        // top (+Y)
        if (pointIsOnQuad(corners[2], corners[3], corners[7], corners[6], pickPt)) return 4;
        // bottom (-Y)
        if (pointIsOnQuad(corners[0], corners[4], corners[5], corners[1], pickPt)) return 1;
        return -1;
    }

    /*
     * Insert a renderer to this visualizer
     * Add it after all renderers of lower render order
     */
    int Visualizer::insertRenderer(Renderer * ren, int newOrder)
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

#ifdef DEAD
        for (int i = _renderer.size() - 1; i >= 0; i--) { delete _renderer[i]; }
#endif

        _renderOrder.clear();
        _renderer.clear();
    }
    /*
     * Remove renderer of specified renderParams
     */
    bool Visualizer::RemoveRenderer(Renderer * ren)
    {
        int i;

        // get it from the renderer list, and delete it:
        bool found = false;
        for (i = 0; i < _renderer.size(); i++) {
            if (_renderer[i] != ren) continue;
#ifdef DEAD
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
        int                    nLights = vpParams->getNumLights();
        float                  lightDirs[3][4];
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

            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, vpParams->getExponent());
            lmodel_ambient[0] = lmodel_ambient[1] = lmodel_ambient[2] = vpParams->getAmbientCoeff();
            // All the geometry will get a white specular color:
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specColor);
            glLightfv(GL_LIGHT0, GL_POSITION, lightDirs[0]);

            specLight[0] = specLight[1] = specLight[2] = vpParams->getSpecularCoeff(0);

            diffLight[0] = diffLight[1] = diffLight[2] = vpParams->getDiffuseCoeff(0);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, diffLight);
            glLightfv(GL_LIGHT0, GL_SPECULAR, specLight);
            glLightfv(GL_LIGHT0, GL_AMBIENT, ambColor);
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
            // Following has unpleasant effects on flow line lighting
            // glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

            glEnable(GL_LIGHT0);
            if (printOpenGLError()) return -1;
            if (nLights > 1) {
                if (printOpenGLError()) return -1;
                glLightfv(GL_LIGHT1, GL_POSITION, lightDirs[1]);
                specLight[0] = specLight[1] = specLight[2] = vpParams->getSpecularCoeff(1);
                diffLight[0] = diffLight[1] = diffLight[2] = vpParams->getDiffuseCoeff(1);
                glLightfv(GL_LIGHT1, GL_DIFFUSE, diffLight);
                glLightfv(GL_LIGHT1, GL_SPECULAR, specLight);
                glLightfv(GL_LIGHT1, GL_AMBIENT, ambColor);
                glEnable(GL_LIGHT1);

            } else {
                glDisable(GL_LIGHT1);
                if (printOpenGLError()) return -1;
            }
            if (nLights > 2) {
                glLightfv(GL_LIGHT2, GL_POSITION, lightDirs[2]);
                specLight[0] = specLight[1] = specLight[2] = vpParams->getSpecularCoeff(2);
                diffLight[0] = diffLight[1] = diffLight[2] = vpParams->getDiffuseCoeff(2);
                glLightfv(GL_LIGHT2, GL_DIFFUSE, diffLight);
                glLightfv(GL_LIGHT2, GL_SPECULAR, specLight);
                glLightfv(GL_LIGHT2, GL_AMBIENT, ambColor);
                glEnable(GL_LIGHT2);
                if (printOpenGLError()) return -1;
            } else {
                glDisable(GL_LIGHT2);
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
        double temp[3];

        // Window height is subtended by viewing angle (45 degrees),
        // at viewer distance (dist from camera to view center)
        const VizFeatureParams *vfParams = getActiveVizFeatureParams();
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
    }
    ViewpointParams *Visualizer::getActiveViewpointParams() const { return m_paramsMgr->GetViewpointParams(m_winName); }

    RegionParams *Visualizer::getActiveRegionParams() const { return m_paramsMgr->GetRegionParams(m_winName); }

    VizFeatureParams *Visualizer::getActiveVizFeatureParams() const { return m_paramsMgr->GetVizFeatureParams(m_winName); }

#ifdef DEAD
    void Visualizer::resetTrackball()
    {
        if (m_trackBall) delete m_trackBall;
        m_trackBall = new Trackball();
    }
#endif

#ifdef DEAD
    void Visualizer::renderManip()
    {
        // render the region manipulator, if in region mode, and active visualizer, or region shared
        // with active visualizer.
        if (MouseModeParams::GetCurrentMouseMode() == MouseModeParams::regionMode) {
            if ((windowIsActive() || (!getActiveRegionParams()->IsLocal() && activeWinSharesRegion()))) {
                TranslateStretchManip *regionManip = getManip(Params::_regionParamsTag);
                regionManip->setParams(getActiveRegionParams());
                regionManip->render();
            }
        }
        // Render other manips, if we are in appropriate mode:
        // Note: Other manips don't have shared and local to deal with:
        else if ((MouseModeParams::GetCurrentMouseMode() != MouseModeParams::navigateMode) && windowIsActive()) {
            int mode = MouseModeParams::GetCurrentMouseMode();

            string                 tag = MouseModeParams::getModeTag(mode);
            TranslateStretchManip *manip = _manipHolder[mode];

            RenderParams *p = (RenderParams *)_paramsMgr->GetCurrentParams(_winNum, tag);
            if (!p) return;
            manip->setParams(p);
            manip->render();
            int manipType = MouseModeParams::getModeManipType(mode);
            // For various manips with window, render 3D cursor too
            //(Not implemented yet)
            if (manipType > 1) {
                //		const double *localPoint = p->getSelectedPointLocal();

                //		draw3DCursor(localPoint);
            }
        }
    }
#endif

    int Visualizer::captureImage(string filename)
    {
        ViewpointParams *vpParams = getActiveViewpointParams();

        size_t width, height;
        vpParams->GetWindowSize(width, height);

        // Turn off the single capture flag
        _imageCaptureEnabled = false;
        string suffix = filename.substr(filename.length() - 4, 4);

        FILE *jpegFile = NULL;
        TIFF *tiffFile = NULL;
        if (suffix == ".tif") {
            tiffFile = TIFFOpen((const char *)filename.c_str(), "wb");
            if (!tiffFile) {
                SetErrMsg("Image Capture Error: Error opening output Tiff file: %s", (const char *)filename.c_str());
                return -1;
            }
        } else {
            // open the jpeg file:
            jpegFile = fopen((const char *)filename.c_str(), "wb");
            if (!jpegFile) {
                SetErrMsg("Image Capture Error: Error opening output Jpeg file: %s", (const char *)filename.c_str());
                return -1;
            }
        }
        // Get the image buffer
        unsigned char *buf = new unsigned char[3 * width * height];
        // Use openGL to fill the buffer:
        if (!getPixelData(buf)) {
            // Error!
            SetErrMsg("Image Capture Error; error obtaining GL data");
            delete[] buf;
            return -1;
        }

        // Now call the Jpeg or tiff library to compress and write the file
        //
        if (suffix == ".jpg") {
            // int quality = getJpegQuality();
            int quality = 99;
            int rc = write_JPEG_file(jpegFile, width, height, buf, quality);
            if (rc) {
                // Error!
                SetErrMsg("Image Capture Error; Error writing jpeg file %s", (const char *)filename.c_str());
                delete[] buf;
                return -1;
            }
        } else {    // capture the tiff file, one scanline at a time
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

        // if (front)
        //
        // glReadBuffer(GL_FRONT);
        //
        // else
        //  {
        glReadBuffer(GL_BACK);
        //  }
        glDisable(GL_SCISSOR_TEST);

        // Turn off texturing in case it is on - some drivers have a problem
        // getting / setting pixels with texturing enabled.
        glDisable(GL_TEXTURE_2D);

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
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        for (int i = 0; i < _renderer.size(); i++) {
            // If a renderer is not initialized, or if its bypass flag is set, then don't render.
            // Otherwise push and pop the GL matrix stack, and all attribs
#ifdef DEAD
            if (_renderer[i]->isInitialized() && !(_renderer[i]->doAlwaysBypass(timeStep))) {
#endif
                _renderer[i]->renderColorbar();
            }
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
        }

        void Visualizer::incrementPath(string & s)
        {
            // truncate the last 4 characters (remove .tif or .jpg)
            string s1 = s.substr(0, s.length() - 4);
            string s_end = s.substr(s.length() - 4);
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
