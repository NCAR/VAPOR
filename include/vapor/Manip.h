//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//					
//	File:		Manip.h 
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2005
//
//	Description:	Defines the pure virtual Manip class
//		Subclasses of this class provide in-scene manipulators
//		for positioning and setting properties of objects
//		in the scene.

#ifndef MANIP_H
#define MANIP_H
//Handle diameter in pixels:
#define HANDLE_DIAMETER 15
#include <vapor/common.h>
namespace VAPoR {

class Visualizer;
class DataStatus;
//! \class Manip
//! \ingroup Public_Render
//! \brief A class that supports manipulators in in a VAPOR Visualizer
//! \author Alan Norton
//! \version 3.0
//! \date    July 2015
//!
//! Manip class is a pure virtual class that supports 
//! manipulators in the VAPOR Visualizer scene
//! Currently two subclasses, TranslateStretchManip and TranslateRotateManip are supported.
//! TranslateStretchManip is used for axis-aligned manipulators, TranslateRotateManip supports
//! manipulators that can be rotated to arbitrary orientation.
//! To use a Manip, programmers must invoke MouseModeParams::RegisterMouseMode() for the
//! Params class associated with the Manip.  
//! This is called in either MouseModeParams::RegisterMouseModes() 
//! [for built-in Manips] or VAPoR::InstallExtensionMouseModes() [for Manips on extension Params classes]
//! 
class RENDER_API Manip {

public:
	Manip(Visualizer* win) {_vis = win;}
	virtual ~Manip(){}

	//! Pure virtual function renders the geometry of the Manip.
	virtual void render()= 0;

	//! Obtain the Params instance currently associated with the Manip.
	//! \return Params* currently associated Params instance
	Params* getParams() {return _params;}
	
	//! Specify the Params instance that is associated with the Manip
	//! \param[in] Params instance associated with Manip
	void setParams(Params* p) {_params = p;}

	//! Pure virtual method: Determine which handle (if any) is under mouse 
	//! \param[in] mouse coordinates in screen
	//! \param[in] boxExtents are extents of full box to which the handles are attached
	//! \param[out] handleMid is the coordinates of the center of the selected handle (if selected).
	//! \return handle index, or -1 if no intersection
	virtual int mouseIsOverHandle(double screenCoords[2], double* boxExtents,  double handleMid[3]) = 0;

	//! Pure virtual function, invoked when the mouse button is released
	//! \param[in] screenCoords are coordinates of mouse at the time it is released
	virtual void mouseRelease(float screenCoords[2]) = 0;

	//! Pure virtual function, indicates that the mouse has been pressed over a handle, so is currently dragging the handle
	//! \return handle index
	virtual int draggingHandle() = 0;
	
	static void setDataStatus(DataStatus* ds) {_dataStatus = ds;}
protected:
	static const float _faceSelectionColor[4];
	static const float _unselectedFaceColor[4];
	
	Params* _params;
	Visualizer* _vis;
	
	//! General utility function for drawing axis-aligned cubes.
	//! \param[in] extents : extents of box to be drawn
	//! \param[in] isSelected indicates if this box is to be drawn with the selection color or not
	void drawCubeFaces(double* extents, bool isSelected);
	
	double _dragDistance;
	int _selectedHandle;
	static DataStatus* _dataStatus;
};
//! \class TranslateStretchManip
//! \ingroup Public_Render
//! \brief A Manip subclass for manipulators that stretch and translate
//! \author Alan Norton
//! \version 3.0
//! \date    July 2015

//! This subclass handles translation and stretching manip.  Works 
//! with ArrowParams (rake).
//! When you slide a handle with the right mouse it stretches the region
class RENDER_API TranslateStretchManip : public Manip {
public:
	TranslateStretchManip(Visualizer* win, Params*p); 
	virtual ~TranslateStretchManip(){}
	virtual void render();

	//! Determine if the mouse is over one of the manip handles.
	//! \param[in] screenCoords x,y screen position of mouse
	//! \param[in] stretchedBoxExtents Extents of manip in stretched coordinates
	//! \param[out] handleMid coordinates of handle selected, in stretched coordinates
	//! \return index of handle, or -1 if none.
	int mouseIsOverHandle(double screenCoords[2], double* stretchedBoxExtents,  double handleMid[3]);

	//! Method to invoke when the mouse has been released after dragging a handle.
	//! \param[in] screenCoords screen coordinates where mouse was released.
	virtual void mouseRelease(float screenCoords[2]);

	//! Determine the current handle index that is being dragged
	//! \return handle index
	virtual int draggingHandle() {return _selectedHandle;}

	//! Method to be invoked when the mouse if first pressed over a handle.
	//! \param[in] handleNum is handle index 0..5
	//! \param[in] camPos is camera coordinates in world (unstretched) coords
	//! \param[in] dirVec is vector from camera to handle in unstretched coords
	//! \param[in] buttonNum indicates which mouse button was pressed.
	//! \param[out] strHandleMid specified 3D coordinates of handle middle in stretched coordinates.
	virtual void captureMouseDown(int handleNum,  const std::vector<double>& camPos, double* dirVec, int buttonNum, double strHandleMid[3]);

	//! Method to be invoked when the mouse is dragging a manip handle, from mouse move event.
	//! \param[in] handleNum index of dragging handle
	//! \param[in] movedRay is vector from camera to handle
	//! \param[in] constrain is true if the manip is constrained to stay inside full domain.
	virtual void slideHandle(int handleNum, double movedRay[3], bool constrain = true);

	//! Method invoked when manip handle drag begins, invoked from VizWin.
	//! \param[in] viz Visualizer associated with this Manip
	//! \param[in] mouseCoords coordinates where mouse is pressed.
	//! \param[in] handle index over which the mouse is pressed
	//! \param[in] p Params that owns the Manipulator
	//! \return true if successful
	bool startHandleSlide(Visualizer* viz, double mouseCoords[2], int handleNum, Params* p);

	//! Set the status of the mouse, invoked when the mouse is pressed or released.
	//! \param downUp true is the mouse is pressed for this manipulator.
	void setMouseDown(bool downUp) {_mouseDownHere = downUp;}

	//! Project the current mouse coordinates to a line in screen space.
	//! The line starts at the mouseDownPosition, and points in the
	//! direction resulting from projecting to the screen the axis 
	//! associated with the dragHandle.  Returns false on error.
	//! Invoked during mouseMoveEvent, uses values of mouseDownPoint(), handleProjVec(), mouseDownHere()
	//! \param[in] mouseCoords coordinates of mouse
	//! \param[out] projCoords coordinates of projected point.
	bool projectPointToLine(double mouseCoords[2], double projCoords[2]);

protected:
	
	virtual void drawBoxFaces();

	//! Method that draws a line connecting a handle to the box center
	//! \param[in] handleNum index of handle
	//! \param[in] handleExtents are extents of handle being drawn
	//! \param[in] extents are the full box extents
	void drawHandleConnector(int handleNum, double* handleExtents, double* extents);

	
	//! Utility to determine the intersection in user coordinates of ray with handle
	//! \param[in] ray is direction of ray
	//! \param[in] cameraPos is current camera position
	//! \param[in] handleNum is index of handle being tested.
	//! \param[in] faceNum is face index
	//! \param[out] intersect is 3D coordinates of intersection point
	//! \return true if there is an intersection
	bool rayHandleIntersect(double ray[3], const std::vector<double>& cameraPos, int handleNum, int faceNum, double intersect[3]);

	//! Utility to construct the extents of a handle
	//! \param[in] handleNum handle index
	//! \param[out] handleExtents the extents of the specified handle
	//! \param[in] octant The octant (0-7) associated with the handle
	//! \param[in] extents The extents of the full box.
	void makeHandleExtents(int handleNum, double* handleExtents, int octant, double* extents);
	
	//! Method to draw the faces of a cube (used as a handle)
	//! \param[in] handleExtents are the extents of the handle
	//! \param[in] isSelected indicates if the handle should be drawn using selected color.
	void drawCubeFaces(double* handleExtents, bool isSelected);
	
	//! Construct the vertices of a handle, to be used to draw its faces.
	//! \param[in] handleNum is the index of the handle
	//! \param[out] handle are the 8 3D coordinates of the vertices of the handle
	//! \param[in] octant is the handle octant (between 0 and 5)
	//! \param[in] boxExtents are the extents of the full box.
	//! \return absolute handle index
	int makeHandleFaces(int handleNum, double handle[8][3], int octant, double* boxExtents);

	bool _isStretching;
	double _handleSizeInScene;
	// screen coords where mouse is pressed:
	float _mouseDownPoint[2];
	// unit vector in direction of handle
	float _handleProjVec[2];
	bool _mouseDownHere;
	double _initialSelectionRay[3];  //Vector from camera to handle, when mouse is initially clicked.
	//Following only used by rotating manip subclasses:
	double _tempRotation;
	int _tempRotAxis;
	
};
//! \class TranslateRotateManip
//! \ingroup Public_Render
//! \brief A Manip subclass for manipulators that stretch, translate, and rotate
//! \author Alan Norton
//! \version 3.0
//! \date    July 2015

//! Subclass of TranslateStretchManip that allows the box to be rotated, and for it to be slid outside
//! the full domain bounds.
class RENDER_API TranslateRotateManip : public TranslateStretchManip {
public:
	TranslateRotateManip(Visualizer* w, Params* p);
	virtual ~TranslateRotateManip(){}
	virtual void render();
	virtual void slideHandle(int handleNum, double movedRay[3], bool constrain = true);
	virtual void mouseRelease(float screenCoords[2]);

	//! Method to support dragging the manip with the thumbwheel
	//! While dragging the thumbwheel call this.
	//! When drag is over, set rot to 0.f
	void setTempRotation(double rot, int axis) {
		_tempRotation = rot; _tempRotAxis = axis;
	}
private:
	virtual void drawBoxFaces();
	double constrainStretch(double currentDist);
	//utility class for handling permutations resulting from rotations of mult of 90 degrees:
	class Permuter {
	public:
		Permuter(double theta, double phi, double psi);
		int permute(int i);
	private:
		int _thetaRot; 
		int _phiRot;
		int _psiRot;
	};

};


};


#endif //MANIP_H
