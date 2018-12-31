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
    int initializeGL(GLManager *glManager);

    //! Set/clear a flag indicating that the trackball has changed the viewpoint.
    //! all the OpenGL rendering is performed in the paintEvent method.  It must be invoked from
    //! a current OpenGL context.
    //! \return zero if successful.
    int paintEvent(bool fast);

    void _applyTransformsForRenderer(Renderer *r);

    //! Issue the OpenGL resize call.  Must be called from an OpenGL context.
    //! \param[in] w Window width in pixels.
    //! \param[in] h Window height in pixels.
    int resizeGL(int w, int h);

    //! Identify the visualizer index associated with this visualizer
    //! \retval visualizer index;
    string getWindowName() const { return m_winName; }

    //! Determine the renderer in the renderer list associated with a specific index
    //! \param[in] index
    //! \return Renderer* associated with the index
    Renderer *getRenderer(int i) const { return _renderers[i]; }

    //! Determine the number of renderers in the renderer list
    //! \return number of renderers
    int getNumRenderers() const { return _renderers.size(); }

    //! Get a renderer
    //! \param[in] RenderParams to be checked for renderer
    //! \return associated RenderParams instance
    Renderer *getRenderer(string type, string instance) const;

    void InsertRenderer(Renderer *ren);

    //! Move the renderer to the front of the render queue
    //! \param[out] Renderer instance that is moved to front
    void moveRendererToFront(const Renderer *ren);
    void moveVolumeRenderersToFront();

    //! Remove a specific renderer from the renderer queue
    //! \param[in] r renderer will be removed
    //! \return true if successful.
    bool RemoveRenderer(Renderer *r);

    //! Determine the approximate size of a pixel in terms of user coordinates,
    //! at the center of the scene.
    double getPixelSize() const;

    //! Turn on or off the image capture enablement.  If on, the next paintEvent will result in capture
    //! Also saves the capture file name
    int setImageCaptureEnabled(bool onOff, string filename)
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
    int setAnimationCaptureEnabled(bool onOff, string filename)
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
    void DrawText(string text, int x, int y, int size, float color[3], int type = 0) { m_vizFeatures->AddText(text, x, y, size, color, type); }

    void ClearText() { m_vizFeatures->ClearText(); }

private:
    //! Render all the colorbars enabled in this visualizer.
    void renderColorbars(int timeStep);

    //! Capture a single image to a file.  Filename must be *.tif or *.jpg
    //! Must be called during paintEvent when captureEnabled has been called.
    //! Will turn off the captureEnabled switch.
    //! \param[in] filename
    //! \return zero if successful
    int captureImage(const std::string &path);

#ifdef VAPOR3_0_0_ALPHA
    //! Render the current active manip, if we are not in navigation mode
    void renderManip();
#endif

    void _loadMatricesFromViewpointParams();

    bool fbSetup();

    //! Definition of OpenGL Vendors
    enum OGLVendorType { UNKNOWN = 0, MESA, NVIDIA, ATI, INTEL };

    //! Identify the OpenGL Vendor
    //! \return OpenGL vendor
    static OGLVendorType GetVendor();

    //! Set up the OpenGL viewport (performed during visualizer initialization
    int setUpViewport();

    // Set up viewing and projection matrices
    //
    void setUpViewMat();

    //! Render all the text objects
    void renderText();

    //! Place the OpenGL directional lights specified in the ViewpointParams
    int placeLights();

    //! Save the current GL modelview matrix in the viewpoint params
    //! \param[in] timestep
    //! \param[in] ViewpointParams  ViewpointParams in which the matrix will be saved
    void saveGLMatrix(int timestep, ViewpointParams *);

    //! Obtain the image from the gl back buffer
    //! \param[out] data is array of rgb byte values, 3 bytes per pixel
    //! \return true if successful
    bool getPixelData(unsigned char *data) const;

    void _deleteFlaggedRenderers();
    int  _initializeNewRenderers();
    void _clearFramebuffer();

    int getCurrentTimestep() const;

    static void incrementPath(string &s);

    const ParamsMgr *   m_paramsMgr;
    const DataStatus *  m_dataStatus;
    string              m_winName;
    GLManager *         _glManager;
    AnnotationRenderer *m_vizFeatures;

    bool   _imageCaptureEnabled;
    bool   _animationCaptureEnabled;
    string _captureImageFile;

    vector<Renderer *> _renderers;
};

};    // namespace VAPoR
