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

struct GLManager;

// class Visualizer;
// class DataStatus;
//! \class Manip
//! \ingroup Public_Render
//! \brief A class that supports manipulators in in a VAPOR Visualizer
//! \author Alan Norton, Scott Pearse
//! \version 3.0
//! \date	July 2015, June 2018
//!
//! Manip class is a pure virtual class that supports
//! manipulators in the VAPOR Visualizer scene
//!
class RENDER_API Manip {
public:
    Manip(GLManager *glManager) : _glManager(glManager){};
    virtual ~Manip() {}

    //! Pure virtual function renders the geometry of the Manip.
    virtual void Render() = 0;

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

protected:
    //! Pure virtual method: Determine which handle (if any) is under mouse
    //! \param[in] mouse coordinates in screen
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

    //! Handles logic for a mouse-press event on one of the manip handles
    //! \param[in] screenCoords is a pixel location on the visualizer window
    /*! \param[in] handleMidpoint is the center of a handle to test that gets
     *	tested against the screenCoords array.
     */
    /*! param[in] buttonNum is the button of the mouse that is clicked, which
     *  determines whether we are moving or stretching the manipulator.
     */
    virtual void _mousePress(double screenCoords[2], double handleMidpoint[3], int buttonNum) = 0;

    //! Handles logic for moving a handle while a mouse button is held down.
    //! param[in] screenCoords is the current position of the mouse cursor
    //! param[in] handleMidpoint is the center of the handle being moved
    virtual void _mouseDrag(double screenCoords[2], double handleMidpoint[3]) = 0;

    /*! Handles logic to set the current extents of the manipulator, held in
     *  the _selection array
     */
    //! param[in] screenCoords - coordinates of the cursor upon mose release
    virtual void _mouseRelease(double screenCoords[2]) = 0;

    //! Adjust the corners of the manipulator extents according to _dragDistance
    //! param[in] corners describes the bounding box of the manipulator
    virtual void _stretchCorners(double corners[8][3]) const = 0;

    GLManager *        _glManager;
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
    TranslateStretchManip(GLManager *glManager);
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
    //! \param[out] handleMid coordinates of handle selected
    //! \return index of handle, or -1 if none.
    int _mouseIsOverHandle(const double screenCoords[2], double handleMid[3]) const;

    //! Method to be invoked when the mouse if first pressed over a handle.
    //! \param[in] handleNum is handle index 0..5
    //! \param[in] buttonNum indicates which mouse button was pressed.
    /*! \param[out] strHandleMid specified 3D coordinates of handle middle in
     * stretched coordinates.
     */
    virtual void _captureMouseDown(int handleNum, int buttonNum, const double strHandleMid[3]);

    /*! Method to be invoked when the mouse is dragging a manip handle, from
     * mouse move event.
     */
    //! \param[in] handleNum index of dragging handle
    //! \param[in] movedRay is vector from camera to handle
    /*! \param[in] constrain is true if the manip is constrained to stay inside
     * full domain.
     */
    virtual void slideHandle(int handleNum, const double movedRay[3], bool constrain);

    //! Method invoked when manip handle drag begins, invoked from VizWin.
    //! \param[in] mouseCoords coordinates where mouse is pressed.
    //! \param[in] handleNum index over which the mouse is pressed
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

    //! Method to render the faces of the manipulator handlebars
    virtual void drawBoxFaces() const;

    //! Method to indicate whether a screencoordinate that is projected
    //! into the scene intersects the coordinates of a cube face
    //! \param[in] cor1 first corner
    //! \param[in] cor2 second corner
    //! \param[in] cor3 third corner
    //! \param[in] cor4 fourth corner
    //! \param[in] pickPt the screen coordinate being projected into the scene
    //! \return true if the projected point intesects the quad
    virtual bool pointIsOnQuad(double cor1[3], double cor2[3], double cor3[3], double cor4[3], const double pickPt[2]) const;

    //! Method to indicate whether a screencoordinate that is projected
    //! into the scene intersects the coordinates of a 3D cube
    //! \param[in] corners The corners constraining the extents of a handlebar
    //! \param[in] pickPt the screen coordinate being projected into the scene
    virtual int pointIsOnBox(double corners[8][3], const double pkPt[2]) const;

    double getPixelSize() const;

    //! Method to scale, translate, or rotate the coordinates of the manipulator
    //! param[in] - the current dataset or renderer transform to operate upon
    void transformMatrix(VAPoR::Transform *transform);

    //! Method to remove the scaling of the manipulator extents
    //! param[in] extents are the current region of the manipulator
    void deScaleExtents(double *extents) const;

    //! Method to remove the scaling of the manipulator extents
    //! param[in] extents are the current region of the manipulator
    void deScaleExtents(double extents[8][3]) const;

    //! Method to apply the inverse-scaling factor to the worldHandleDiameter
    //! used in makeHandleExtents()
    //! param[in] worldHandleDiameter - the size of the manipulator handle
    //! param[in] axis - The axis to descale the manipulator handle size upon
    void deScaleScalarOnAxis(float &scalar, int axis) const;

    //! Debug tool to draw the hitbox of a manipulator handle.  This should
    //! align with the window-coordinates of the actual handle.
    //! param[in] winCoord1 - first corner of a 2D quad on the screen
    //! param[in] winCoord2 - secdond corner of a 2D quad on the screen
    //! param[in] winCoord3 - third corner of a 2D quad on the screen
    //! param[in] winCoord4 - fourth corner of a 2D quad on the screen
    void drawHitBox(double winCoord1[2], double winCoord2[2], double winCoord3[2], double winCoord4[2]) const;

    //! Method that draws a line connecting a handle to the box center
    //! \param[in] handleNum index of handle
    //! \param[in] handleExtents are extents of handle being drawn
    //! \param[in] extents are the full box extents
    void drawHandleConnector(int handleNum, double *handleExtents, double *extents);

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

    void _mousePress(double screenCoords[2], double handleMidpoint[3], int buttonNum);
    void _mouseDrag(double screenCoords[2], double handleMidpoint[3]);
    void _mouseRelease(double screenCoords[2]);
    void _stretchCorners(double corners[8][3]) const;
    void _translateCorners(double corners[8][3]) const;
    void _moveMinusXCorners(double corners[8][3]) const;
    void _moveMinusYCorners(double corners[8][3]) const;
    void _moveMinusZCorners(double corners[8][3]) const;
    void _movePlusXCorners(double corners[8][3]) const;
    void _movePlusYCorners(double corners[8][3]) const;
    void _movePlusZCorners(double corners[8][3]) const;
    void _constrainExtents();

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
