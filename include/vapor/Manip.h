//************************************************************************
//									*
//			 Copyright (C)  2004				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Manip.h
//
//	Author:		Alan Norton
//				Scott Pearse
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:	Vapor3 implementation - May 2018
//			Vapor2 implementation - November 2005
//
//	Description:	Defines the pure virtual Manip class
//		Subclasses of this class provide in-scene manipulators
//		for positioning and setting properties of objects
//		in the scene.

#ifndef MANIP_H
#define MANIP_H

#include <vapor/glutil.h>
#include "vapor/Transform.h"

// Handle diameter in pixels:
#define HANDLE_DIAMETER 15
//#define HANDLE_DIAMETER 3
namespace VAPoR {

// class Visualizer;
// class DataStatus;
//! \class Manip
//! \ingroup Public_Render
//! \brief A class that supports manipulators in in a VAPOR Visualizer
//! \author Alan Norton
//! \version 3.0
//! \date	July 2015
//!
//! Manip class is a pure virtual class that supports
//! manipulators in the VAPOR Visualizer scene
//! Currently two subclasses, TranslateStretchManip and TranslateRotateManip are
//! supported.
//! TranslateStretchManip is used for axis-aligned manipulators.
//! TranslateRotateManip supports manipulators that can be rotated to arbitrary
//! orientation.  To use a Manip, programmers must invoke
//! MouseModeParams::RegisterMouseMode() for the Params class associated with the
//! Manip.
//! This is called in either MouseModeParams::RegisterMouseModes()
//! [for built-in Manips] or VAPoR::InstallExtensionMouseModes() [for Manips on
//! extension Params classes]
//!
class RENDER_API Manip {
public:
    // Manip(Visualizer* win) {_vis = win;}
    Manip(){};
    virtual ~Manip() {}

    //! Pure virtual function renders the geometry of the Manip.
    virtual void render() = 0;

    //! Update the box manipulator's extents
    //! \param[in] llc : The lower-left corner to update the manipulator with
    //! \param[in] urc : The upper-right corner to update the manipulator with
    //! \param[in] minExtents : The minimum extents that the manipulator can draw to
    //! \param[in] maxExtents : The maximum extents that the manipulator can draw to
    /*! \param[in] projectionMatrix: The current ProjectionMatrix in the Params
     *	database
     */
    /*! \param[in] modelViewMatrix: The current ModelView matrix in the Params
     *  database
     */
    //! \param[in] windowSize: The current window size of the Visualizer
    /*! \param[in] rpTransform: The current scene transform used by the current
     *  RenderParams
     */
    /*! \param[in] dmTransform: The current scene transform used by the current
     *  DataMgr
     */
    /*! \param[in] constrain: Indicates whether the manipulator box is bound by
     *  domain extents
     */
    virtual void Update(std::vector<double> llc, std::vector<double> urc, std::vector<double> minExtents, std::vector<double> maxExtents, std::vector<double> cameraPosition,
                        double modelViewMatrix[16], double projectionMatrix[16], std::vector<int> windowSize, VAPoR::Transform *rpTransform, VAPoR::Transform *dmTransform, bool constrain) = 0;

    //! Notify that manipulator that is being moved with the mouse
    //! \param[in] buttonNum - The mouse button being used to move the manipulator
    //! \param[in] screenCoords - The current coordinates of the mouse cursor
    virtual bool MouseEvent(int buttonNum, std::vector<double> screenCoords, double handleMidpoint[3], bool release = false) = 0;

    //! Method to retrieve the manipulator's current extents
    //! \param[out] llc - The lower-left coordinates of the manipulator
    //! \param[out] urc - The upper-right coordinates of the manipulator
    virtual void GetBox(std::vector<double> &llc, std::vector<double> &urc) const = 0;

private:
    //! Pure virtual method: Determine which handle (if any) is under mouse
    //! \param[in] mouse coordinates in screen
    /*! \param[in] boxExtents are extents of full box to which the handles are
     * attached
     */
    /*! \param[out] handleMid is the coordinates of the center of the selected
     * handle (if selected).
     */
    //! \return handle index, or -1 if no intersection
    virtual int _mouseIsOverHandle(const double screenCoords[2], double handleMid[3]) const = 0;

    //! General utility function for drawing axis-aligned cubes.
    //! \param[in] extents : extents of box to be drawn
    /*! \param[in] isSelected indicates if this box is to be drawn with the
     * selection color or not
     */
    void _drawCubeFaces(double *extents, bool isSelected);

    virtual void _mousePress(double screenCoords[2], double handleMidpoint[3], int buttonNum) = 0;

    virtual void _mouseDrag(double screenCoords[2], double handleMidpoint[3]) = 0;

    virtual void _mouseRelease(double screenCoords[2]) = 0;

    virtual void _stretchCorners(double corners[8][3]) const = 0;

    int                _buttonNum;
    int                _selectedHandle;
    double             _dragDistance;
    double             _handleMid[3];
    double             _selection[6];
    double             _extents[6];
    double             _cameraPosition[3];
    static const float _faceSelectionColor[4];
    static const float _unselectedFaceColor[4];
};

//! \class TranslateStretchManip
//! \ingroup Public_Render
//! \brief A Manip subclass for manipulators that stretch and translate
//! \author Alan Norton, Scott Pearse
//! \version 3.0
//! \date	July 2015, June 2018

//! This subclass handles translation and stretching manip.  Works
//! with ArrowParams (rake).
//! When you slide a handle with the right mouse it stretches the region
class RENDER_API TranslateStretchManip : public Manip {
public:
    TranslateStretchManip();
    virtual ~TranslateStretchManip() {}
    virtual void Render();

    //! @copydoc Manip::Update(std::vector<double>, std::vector<double> std::vector<double>, std::vector<double>)
    virtual void Update(std::vector<double> llc, std::vector<double> urc, std::vector<double> minExtents, std::vector<double> maxExtents, std::vector<double> cameraPosition,
                        double modelViewMatrix[16], double projectionMatrix[16], std::vector<int> windowSize, VAPoR::Transform *rpTransform, VAPoR::Transform *dmTransform, bool constrain);

    //! @copydoc Manip::MoveEvent(int, std::vector<double>)
    virtual bool MouseEvent(int buttonNum, std::vector<double> screenCoords, double handleMidpoint[3], bool release = false);

    //! @copydoc Manip::GetBox(std::vector<double>, std::vector<double>);
    virtual void GetBox(std::vector<double> &llc, std::vector<double> &urc) const;

private:
    //! Determine if the mouse is over one of the manip handles.
    //! \param[in] screenCoords x,y screen position of mouse
    //! \param[in] stretchedBoxExtents Extents of manip in stretched coordinates
    //! \param[out] handleMid coordinates of handle selected
    //! \return index of handle, or -1 if none.
    int mouseIsOverHandle(const double screenCoords[2], double handleMid[3]) const;

    //! Determine the current handle index that is being dragged
    //! \return handle index
    virtual int draggingHandle() const { return _selectedHandle; }

    //! Method to be invoked when the mouse if first pressed over a handle.
    //! \param[in] handleNum is handle index 0..5
    //! \param[in] camPos is camera coordinates in world (unstretched) coords
    //! \param[in] dirVec is vector from camera to handle in unstretched coords
    //! \param[in] buttonNum indicates which mouse button was pressed.
    /*! \param[out] strHandleMid specified 3D coordinates of handle middle in
     * stretched coordinates.
     */
    virtual void captureMouseDown(int handleNum, int buttonNum, const double strHandleMid[3]);

    /*! Method to be invoked when the mouse is dragging a manip handle, from
     * mouse move event.
     */
    //! \param[in] handleNum index of dragging handle
    //! \param[in] movedRay is vector from camera to handle
    /*! \param[in] constrain is true if the manip is constrained to stay inside
     * full domain.
     */
    // virtual void slideHandle(int handleNum, double movedRay[3]);
    virtual void slideHandle(int handleNum, const double movedRay[3], bool constrain);

    //! Method invoked when manip handle drag begins, invoked from VizWin.
    //! \param[in] viz Visualizer associated with this Manip
    //! \param[in] mouseCoords coordinates where mouse is pressed.
    //! \param[in] handle index over which the mouse is pressed
    //! \param[in] p Params that owns the Manipulator
    //! \return true if successful
    bool startHandleSlide(const double mouseCoords[2], int handleNum);

    /*! Set the status of the mouse, invoked when the mouse is pressed or
     * released.
     */
    //! \param downUp true is the mouse is pressed for this manipulator.
    void setMouseDown(bool downUp) { _mouseDownHere = downUp; }

    //! Project the current mouse coordinates to a line in screen space.
    //! The line starts at the mouseDownPosition, and points in the
    //! direction resulting from projecting to the screen the axis
    //! associated with the dragHandle.  Returns false on error.
    //! Invoked during mouseMoveEvent, uses values of mouseDownPoint(),
    //! handleProjVec(), mouseDownHere()
    //! \param[in] mouseCoords coordinates of mouse
    //! \param[out] projCoords coordinates of projected point.
    bool projectPointToLine(const double mouseCoords[2], double projCoords[2]);

    //! Determine a vector associated with a pixel, pointing from the
    //! camera, through the pixel into the scene to a manip handle.  Uses OpenGL
    //! screencoords.
    //! I.e. y = 0 at bottom.  Returns false on failure.  Used during mouse Events.
    //! \param[in] winCoords  pixel coordinates (converted to double)
    //! \param[out] dirVec resulting vector, from camera to handle
    //! \param[in] strHandleMid is middle of handle in stretched coordinates.
    //! \return true if successful
    bool pixelToVector(double winCoords[2], double dirVec[3], const double strHandleMid[3]);

    virtual void drawBoxFaces() const;
    virtual bool pointIsOnQuad(double cor1[3], double cor2[3], double cor3[3], double cor4[3], const double pickPt[2]) const;
    virtual int  pointIsOnBox(double corners[8][3], const double pkPt[2]) const;
    double       getPixelSize() const;
    void         transformMatrix(VAPoR::Transform *transform);
    void         deScaleExtents(double *extents) const;
    void         deScaleExtents(double extents[8][3]) const;
    void         drawHitBox(double winCoord1[2], double winCoord2[2], double winCoord3[2], double winCoord4[2]);

    //! Method that draws a line connecting a handle to the box center
    //! \param[in] handleNum index of handle
    //! \param[in] handleExtents are extents of handle being drawn
    //! \param[in] extents are the full box extents
    void drawHandleConnector(int handleNum, double *handleExtents, double *extents);

    //! Utility to determine the intersection in user coordinates of ray with handle
    //! \param[in] ray is direction of ray
    //! \param[in] cameraPos is current camera position
    //! \param[in] handleNum is index of handle being tested.
    //! \param[in] faceNum is face index
    //! \param[out] intersect is 3D coordinates of intersection point
    //! \return true if there is an intersection
    bool rayHandleIntersect(double ray[3], const std::vector<double> &cameraPos, int handleNum, int faceNum, double intersect[3]);

    //! Utility to construct the extents of a handle
    //! \param[in] handleNum handle index
    //! \param[out] handleExtents the extents of the specified handle
    //! \param[in] octant The octant (0-7) associated with the handle
    //! \param[in] extents The extents of the full box.
    void makeHandleExtents(int handleNum, double *handleExtents, int octant, double *extents);

    //! Method to draw the faces of a cube (used as a handle)
    //! \param[in] handleExtents are the extents of the handle
    /*! \param[in] isSelected indicates if the handle should be drawn using
     * selected color.
     */
    void drawCubeFaces(double *handleExtents, bool isSelected);

    //! Construct the vertices of a handle, to be used to draw its faces.
    //! \param[in] handleNum is the index of the handle
    //! \param[out] handle are the 8 3D coordinates of the handle's vertices
    //! \param[in] octant is the handle octant (between 0 and 5)
    //! \param[in] boxExtents are the extents of the full box.
    //! \return absolute handle index
    int makeHandleFaces(int handleNum, double handle[8][3], int octant, const double *boxExtents) const;

    //! Project a 3D point (in user coord system) to window coords.
    /*! Return true if in front of camera.  Used by pointIsOnQuad, as well as
     * in building Axis labels.
     */
    //! \param[in] userCoords[3] coordinates of point
    //! \param[out] winCoords[2] window coordinates of projection
    //! \return true if point is in front of camera.
    bool _projectPointToWin(const double cubeCoords[3], double winCoords[2]) const;

    void mousePress(double screenCoords[2], double handleMidpoint[3], int buttonNum);
    void mouseDrag(double screenCoords[2], double handleMidpoint[3]);
    void mouseRelease(double screenCoords[2]);
    void stretchCorners(double corners[8][3]) const;
    void translateCorners(double corners[8][3]) const;
    void moveMinusXCorners(double corners[8][3]) const;
    void moveMinusYCorners(double corners[8][3]) const;
    void moveMinusZCorners(double corners[8][3]) const;
    void movePlusXCorners(double corners[8][3]) const;
    void movePlusYCorners(double corners[8][3]) const;
    void movePlusZCorners(double corners[8][3]) const;
    void constrainExtents();

    bool              _isStretching;
    bool              _constrain;
    double            _handleSizeInScene;
    double            _cameraPosition[3];
    double            _modelViewMatrix[16];
    double            _projectionMatrix[16];
    std::vector<int>  _windowSize;
    VAPoR::Transform *_rpTransform;
    VAPoR::Transform *_dmTransform;

    // screen coords where mouse is pressed:
    float _mouseDownPoint[2];

    // unit vector in direction of handle
    float _handleProjVec[2];

    bool _mouseDownHere;

    // Vector from camera to handle, when mouse is initially clicked.
    double _initialSelectionRay[3];

    // Following only used by rotating manip subclasses:
    double _tempRotation;
    int    _tempRotAxis;
};
};    // namespace VAPoR

#endif    // MANIP_H
