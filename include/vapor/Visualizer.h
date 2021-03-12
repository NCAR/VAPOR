//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Visualizer.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		September 2013
//
//	Description:  Definition of Visualizer class:
//	A Visualizer object is associated with each visualization window.  It contains
//  information required for rendering, performs
//	navigation and resize, and defers drawing to the viz window's list of
//	registered renderers.

#pragma once

#include <map>
#include <vapor/DataStatus.h>
#include <vapor/ParamsMgr.h>
#include <vapor/Renderer.h>
#include <vapor/AnnotationRenderer.h>
#include <vapor/Framebuffer.h>

namespace VAPoR {

//! \class Visualizer
//! \ingroup Public_Render
//! \brief A class for performing OpenGL rendering in VAPOR GUI Window
//! \author Alan Norton
//! \version 3.0
//! \date    October 2013
//!
//!
//! The Visualizer class performs OpenGL rendering for the main VAPOR visualizers.  The
//! Visualizer class is not a GL Window itself, however it will issue the OpenGL calls to
//! perform rendering in a context that is already current.

class RENDER_API Visualizer : public MyBase {
public:
    Visualizer(const ParamsMgr *pm, const DataStatus *dataStatus, string winName);
    ~Visualizer();

    //! Method that returns the ViewpointParams that is active in this window.
    //! \retval ViewpointParams* current active ViewpointParams
    ViewpointParams *getActiveViewpointParams() const;

    //! Method that returns the RegionParams that is active in this window.
    //! \retval RegionParams* current active RegionParams
    RegionParams *getActiveRegionParams() const;

    //! Method that returns the AnnotationParams that is active in this window.
    //! \retval AnnotationParams* current active AnnotationParams
    AnnotationParams *getActiveAnnotationParams() const;

    //! Method to initialize GL rendering.  Must be called from a GL context.
    //! \param[in] glManager A pointer to a GLManager
    int InitializeGL(GLManager *glManager);

    //! Set/clear a flag indicating that the trackball has changed the viewpoint.
    //! all the OpenGL rendering is performed in the paintEvent method.  It must be invoked from
    //! a current OpenGL context.
    //! \return zero if successful.
    int paintEvent(bool fast);

    //! Issue the OpenGL resize call.  Must be called from an OpenGL context.
    //! \param[in] w Window width in pixels.
    //! \param[in] h Window height in pixels.
    int resizeGL(int w, int h);

    //! Identify the visualizer index associated with this visualizer
    //! \retval visualizer index;
    string GetWindowName() const { return _winName; }

    //! Determine the number of renderers in the renderer list
    //! \return number of renderers
    int GetNumRenderers() const { return _renderers.size(); }

    //! \note A render params instance must have been previously created for
    //! this renderer.
    //
    int CreateRenderer(string dataSetName, string renderType, string renderName);

    //! Flag a renderer for destruction.
    //!
    //! \param[in] hasOpenGLContext If true it is the callers job to ensure that the
    //! OpenGL Context for the window \p winName is active. In this case the renderer
    //! is destroyed immediately. If false the renderer is queue'd for later destruction
    //! when \p winName has an active OpenGL context.
    //
    void DestroyRenderer(string renderType, string renderName, bool hasOpenGLContext);

    //! Flag all rendereres for destruction.
    //!
    //! \param[in] hasOpenGLContext If true it is the callers job to ensure that the
    //! OpenGL Context for the window \p winName is active. In this case the renderer
    //! is destroyed immediately. If false the renderer is queue'd for later destruction
    //! when \p winName has an active OpenGL context.
    //
    void DestroyAllRenderers(bool hasOpenGLContext);

    bool HasRenderer(string renderType, string renderName) const;

    //! Move the renderer to the front of the render queue
    //! \param[out] Renderer instance that is moved to front
    void MoveRendererToFront(string renderType, string renderName);
    void MoveRenderersOfTypeToFront(const std::string &type);

    //! Determine the approximate size of a pixel in terms of user coordinates,
    //! at the center of the scene.
    double getPixelSize() const;

    //! Turn on or off the image capture enablement.  If on, the next paintEvent will result in capture
    //! Also saves the capture file name
    int SetImageCaptureEnabled(bool onOff, string filename)
    {
        if (_animationCaptureEnabled) {
            SetErrMsg("Image capture concurrent with Animation Capture\n");
            return -1;
        }
        _imageCaptureEnabled = onOff;
        if (onOff)
            _captureImageFile = filename;
        else
            _captureImageFile = "";
        return 0;
    }

    //! Turn on or off the animation capture enablement.  If on, all paintEvents will result in capture
    //! until it is turned off
    int SetAnimationCaptureEnabled(bool onOff, string filename)
    {
        if (_imageCaptureEnabled) {
            SetErrMsg("Image capture concurrent with Animation Capture\n");
            return -1;
        }
        if (_animationCaptureEnabled == onOff) {
            SetErrMsg("Animation capture in incorrect state\n");
            return -1;
        }
        _animationCaptureEnabled = onOff;
        if (onOff)
            _captureImageFile = filename;
        else
            _captureImageFile = "";
        return 0;
    }

    //! Draw a text banner at x, y coordinates
    //
    void DrawText(string text, int x, int y, int size, float color[3], int type = 0) { _vizFeatures->AddText(text, x, y, size, color, type); }

    void DrawTextNormalizedCoords(string text, float x, float y, int size, float color[3], int type = 0) { _vizFeatures->AddTextNormalizedCoords(text, x, y, size, color, type); }

    void ClearText() { _vizFeatures->ClearText(); }

    //! Force each renderer to empty its cache
    //!
    //! This method calls ClearCache on every renderer
    //
    void ClearRenderCache();

private:
    //! Render all the colorbars enabled in this visualizer.
    void _renderColorbars(int timeStep);

    //! Capture a single image to a file.  Filename must be *.tif or *.jpg
    //! Must be called during paintEvent when captureEnabled has been called.
    //! Will turn off the captureEnabled switch.
    //! \param[in] filename
    //! \return zero if successful
    int _captureImage(std::string path);

    void _loadMatricesFromViewpointParams();

    //! Definition of OpenGL Vendors
    enum GLVendorType { UNKNOWN = 0, MESA, NVIDIA, ATI, INTEL };

    //! Identify the OpenGL Vendor
    //! \return OpenGL vendor
    static GLVendorType GetVendor();

    //! Place the OpenGL directional lights specified in the ViewpointParams
    int _configureLighting();

    //! Obtain the image from the gl back buffer
    //! \param[out] data is array of rgb byte values, 3 bytes per pixel
    //! \return true if successful
    bool _getPixelData(unsigned char *data) const;

    void _deleteFlaggedRenderers();
    int  _initializeNewRenderers();
    void _clearActiveFramebuffer(float r, float g, float b) const;
    void _applyDatasetTransformsForRenderer(Renderer *r);

    int _getCurrentTimestep() const;

    static void _incrementPath(string &s);

    Renderer *_getRenderer(string type, string instance) const;

    const ParamsMgr *   _paramsMgr;
    const DataStatus *  _dataStatus;
    string              _winName;
    GLManager *         _glManager;
    AnnotationRenderer *_vizFeatures;

    bool _insideGLContext;    // This is only to make sure we don't call certain functions when they are not supposed to be called. In some situations this variable will be set to true incorrectly. In
                              // those cases there is already some other error so it doesn't matter.
    bool   _imageCaptureEnabled;
    bool   _animationCaptureEnabled;
    string _captureImageFile;

    vector<Renderer *> _renderers;
    vector<Renderer *> _renderersToDestroy;

    Framebuffer  _framebuffer;
    unsigned int _screenQuadVAO;
    unsigned int _screenQuadVBO;
};

};    // namespace VAPoR
