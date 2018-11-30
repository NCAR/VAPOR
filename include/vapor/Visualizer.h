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

#ifndef Visualizer_H
#define Visualizer_H

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

    //! Apply user defined transforms to the current renderer being drawn
    //! \param[in] renIndex The index of the current renderer being drawn,
    //! referring to the _renderer list
    void applyTransforms(int renIndex);

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
    Renderer *getRenderer(int i) const { return _renderer[i]; }

    //! Determine the number of renderers in the renderer list
    //! \return number of renderers
    int getNumRenderers() const { return _renderer.size(); }

    //! Get a renderer
    //! \param[in] RenderParams to be checked for renderer
    //! \return associated RenderParams instance
    Renderer *getRenderer(string type, string instance) const;

#ifdef VAPOR3_0_0_ALPHA
    //! Identify the AnnotationRenderer associated with this Visualizer
    //! \return associated RenderParams instance
    AnnotationRenderer *getAnnotationRenderer() { return _vizFeatures; }
#endif

    //! Insert a renderer in the queue using the default (5) render order
    //! \sa Visualizer::insertRenderer
    //! \param[out] Renderer instance that is inserted in the queue
    //! \return position in the renderer queue
    int insertSortedRenderer(Renderer *ren) { return insertRenderer(ren, 5); }

    //! Move the renderer to the front of the render queue
    //! \param[out] Renderer instance that is moved to front
    void moveRendererToFront(const Renderer *ren);

    //! Remove a specific renderer from the renderer queue
    //! \param[in] r renderer will be removed
    //! \return true if successful.
    bool RemoveRenderer(Renderer *r);

    //! Remove (and delete) all the renderers in the renderer queue
    void removeAllRenderers();

    //! Remove all disabled renderers from the queue
    void removeDisabledRenderers();

#ifdef VAPOR3_0_0_ALPHA
    //! Obtain the manip that is associated with a specified Params type
    //! \param[in] tag associated with the Params that owns the manip
    //! \return pointer to the Manip
    TranslateStretchManip *getManip(const std::string &paramTag) const
    {
        int mode = MouseModeParams::getModeFromParams(paramTag);
        return _manipHolder[mode];
    }

#endif

    //! Determine the approximate size of a pixel in terms of user coordinates,
    //! at the center of the scene.
    double getPixelSize() const;

    //! Set the region share flag, indicating whether the active visualizer is sharing
    //! the Region.  This is needed for Manip rendering, and is set whenever the active visualizer
    //! is changed.
    //! \param[in] true if the active visualizer uses the shared (global) Region
    static void setRegionShareFlag(bool regionIsShared) { _regionShareFlag = regionIsShared; }

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

    //! Setup the OpenGL state for rendering
    //! \param[in] timeStep is the timestep associated with the frame to be rendered.
    //! \return 0 if successful.
    int paintSetup(int timeStep);

    bool fbSetup();

    //! Renderers can be added early or late, using a "render Order" parameter.
    //! The order is between 0 and 10; lower order gets rendered first.
    //! Sorted renderers get sorted before each render
    //! To insert a renderer specify the associated RenderParams, the Renderer, and the render order
    //! \param[in] RenderParams associated with the renderer
    //! \param[out] Renderer object constructed from the RenderParams
    //! \param[in] render order
    //! \return position in the renderer queue
    int insertRenderer(Renderer *ren, int order);

    //! Determine if the active visualizer is using the shared Region.
    //! \sa Visualizer::setRegionShareFlag()
    //! \return true if the active visualizer uses the shared Region.
    static bool activeWinSharesRegion() { return _regionShareFlag; }

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

    int getCurrentTimestep() const;

    static void incrementPath(string &s);

    const ParamsMgr *   m_paramsMgr;
    const DataStatus *  m_dataStatus;
    string              m_winName;
    GLManager *         _glManager;
    AnnotationRenderer *m_vizFeatures;
    bool                m_viewpointDirty;

    //! There's a separate manipholder for each window
#ifdef VAPOR3_0_0_ALPHA
    vector<TranslateStretchManip *> _manipHolder;
#endif
    bool   _imageCaptureEnabled;
    bool   _animationCaptureEnabled;
    string _captureImageFile;
    int    _previousTimeStep;
    int    _previousFrameNum;

    vector<Renderer *> _renderer;
    vector<int>        _renderOrder;

    // This flag is true if the active window is sharing the region.
    // If the current window is not active, it will still share the region, if
    // the region is shared, and the active region is shared.
    static bool _regionShareFlag;

    //! Reset the near/far clipping, so that the near and far clipping
    //! planes are wide enough
    //! to view twice the entire region from the current camera position.
    //! If the camera is
    //! inside the doubled region, make the near clipping plane 1% of the
    //! region size.
    //! \param[in] vpParams ViewpointParams* used to determine current
    //! camera position.
    void resetNearFar();
};

};    // namespace VAPoR

#endif    // Visualizer_H
