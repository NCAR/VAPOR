//************************************************************************
//									*
//		     Copyright (C)  2005				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Manip.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2005
//
//	Description:	Implements the Manip class and some of its subclasses
//TOGO:

#include <vapor/glutil.h>	// Must be included first!!!
#include <vapor/Visualizer.h>
#include <vapor/params.h>
#include <vapor/ViewpointParams.h>
#include <vapor/DataStatus.h>

#include "Manip.h"

using namespace VAPoR;
const float Manip::_faceSelectionColor[4] = {0.8f,0.8f,0.0f,0.8f};
const float Manip::_unselectedFaceColor[4] = {0.8f,0.2f,0.0f,0.8f};
DataStatus* Manip::_dataStatus = 0;

//Methods for TranslateStretchManip:
//


TranslateStretchManip::TranslateStretchManip(Visualizer* win, Params* p) : Manip(win) {
	
	setParams(p);
	_selectedHandle = -1;
	_isStretching = false;
	_tempRotation = 0.f;
	_tempRotAxis = -1;
	_handleSizeInScene = 1.;
	_initialSelectionRay[0] = 0.;
	_initialSelectionRay[1] = 0.;
	_initialSelectionRay[2] = 0.;
	_mouseDownHere = false;
}

// Determine if the mouse is over any of the six handles.
// 3D Inputs are in stretched coordinates.
// Test first the 3 handles in front, then the object, then the three in back.
// Return the handle num if we are over one, or -1 if not.
// The boxExtents are the extents of the box being drawn
// Does not take into account drag distance, because the mouse is just being clicked.
//
int TranslateStretchManip::
mouseIsOverHandle(double screenCoords[2], double* boxExtents, double handleMid[3]){
	//Determine if the mouse is over any of the six handles.
	//Test first the 3 handles in front, then the object, then the three in back.
	//The specified getHandle methods must return boxes with prescribed alignment
	// (ctr clockwise around front (+Z) starting at -X, -Y, then clockwise
	// around back (-Z) starting at -x ,-y.
	double handle[8][3];
	
	
	//Get the camera position in stretched coords.  This is needed to determine
	//which handles are in front of the box.
	ViewpointParams* myViewpointParams = _vis->getActiveViewpointParams();
	vector<double> camPos = myViewpointParams->getCameraPosLocal();
#ifdef	DEAD
	vector<double> stretch = _dataStatus->getStretchFactors();
#else
	vector<double> stretch(3,1.0);
#endif
	for (int i=0; i<stretch.size(); i++) {
		camPos[i] *= stretch[i];
	}
	
	//Determine the octant based on camera relative to box center:
	int octant = 0;
	//face identifies the faceNumber (for intersection calc) if there is
	int face, handleNum;
	for (int axis = 0; axis < 3; axis++){
		if (camPos[axis] > 0.5f*(boxExtents[axis]+boxExtents[axis+3])) octant |= 1<<axis;
	}
	for (int sortNum = 0; sortNum < 3; sortNum++){
		handleNum = makeHandleFaces(sortNum, handle, octant, boxExtents);
		if((face = _vis->pointIsOnBox(handle, screenCoords)) >= 0){
			
			//Save the handle middle coordinates:
			for (int i = 0; i<3; i++){
				handleMid[i] = 0.;
				for (int k = 0; k<8; k++)
					handleMid[i] += handle[k][i];
				handleMid[i] /= 8.;
			}
			return handleNum;
		}
	}
	
	
	//Then check backHandles
	for (int sortNum = 3; sortNum < 6; sortNum++){
		handleNum = makeHandleFaces(sortNum, handle, octant, boxExtents);
		if((face = _vis->pointIsOnBox(handle, screenCoords)) >= 0){
			//Save the handle middle coordinates:
			for (int i = 0; i<3; i++){
				handleMid[i] = 0.;
				for (int k = 0; k<8; k++)
					handleMid[i] += handle[k][i];
				handleMid[i] /= 8.;
			}
			return handleNum;
		}
	}
	return -1;
}
//Construct one of 6 cube-handles.  The first 3 (0,1,2) are those in front of the probe 
// the octant (between 0 and 7) is a binary representation of the viewer relative to
// the probe center.  In binary, this value is ZYX, where Z is 1 if the viewer is above,
// 0 if the viewer is below, similarly for Y and X.  The handles are numbered from 0 to 5,
// in the order -X, +X, -Y, +Y, -Z, +Z.  
// 
// We provide the handle in the appropriate sort order, based on the octant
// in which the viewer is located.  The sort position of a handle is either
// N or (5-N) where N is the order sorted on x,y,z axes.  It is 5-N if the
// corresponding bit in the octant is 1.  Octant is between 0 and 7
// with a bit for each axis (Z,Y,X order).
// Return value is the absolute handle index, which may differ from the sort order.
// If you want the faces in absolute order, just specify octant = 0
//
// We make the handles in cube coords, since rendering and picking use that system.
// This requires that we have boxCenter and boxWidth in cube coords.
// HANDLE_DIAMETER is measured in pixels
// These can be calculated from the boxRegion 
int TranslateStretchManip::
makeHandleFaces(int sortPosition, double handle[8][3], int octant, double boxRegion[6]){
	//Identify the axis this handle is on:
	int axis = (sortPosition<3) ? (2-sortPosition) : (sortPosition-3);
	int newPosition = sortPosition;
	if ((octant>>axis) & 1) newPosition = 5 - sortPosition;
	
	//Now create the cube associated with newPosition.  It's just the handle translated
	//in the direction associated with newPosition
	float translateSign = (newPosition > 2) ? 1.f : -1.f;
	for (int vertex = 0; vertex < 8; vertex ++){
		for (int coord = 0; coord<3; coord++){
			//Obtain the coordinate of unit cube corner.  It's either +0.5 or -0.5
			//multiplied by the handle diameter, then translated along the 
			//specific axis corresponding to 
			double fltCoord = (((double)((vertex>>coord)&1) -0.5f)*_handleSizeInScene);
			//First offset it from the probeCenter:
			fltCoord += 0.5f*(boxRegion[coord+3] + boxRegion[coord]);
			//Displace all the c - coords of this handle if this handle is on the c-axis
			if (coord == axis){
				double boxWidth = (boxRegion[coord+3] - boxRegion[coord]);
				//Note we are putting the handle 2 diameters from the box edge
				fltCoord += translateSign*(boxWidth*0.5f + 2.f*_handleSizeInScene);
			}
			handle[vertex][coord] = fltCoord;
		}
	}
	return newPosition;
}
bool TranslateStretchManip::startHandleSlide(Visualizer* viz, double mouseCoords[2], int handleNum, Params* manipParams){
	// When the mouse is first pressed over a handle, 
	// need to save the
	// windows coordinates of the click, as well as
	// calculate a 2D unit vector in the direction of the slide,
	// projected into the window.

	_mouseDownPoint[0] = mouseCoords[0];
	_mouseDownPoint[1] = mouseCoords[1];
	//Get the cube coords of the rotation center:
	
	double boxCtr[3]; 
	double winCoords[2] = {0.,0.};
	double dispCoords[2];
	
	if (handleNum > 2) handleNum = handleNum-3;
	else handleNum = 2 - handleNum;
	double boxExtents[6];
#ifdef	DEAD
	int timestep = viz->getActiveAnimationParams()->GetCurrentTimestep();
#endif
	manipParams->GetBox()->GetStretchedLocalExtents(boxExtents, timestep);
	
	for (int i = 0; i<3; i++){boxCtr[i] = (boxExtents[i] + boxExtents[i+3])*0.5f;}
	// project the boxCtr and one more point, to get a direction vector
	
	if (!viz->projectPointToWin(boxCtr, winCoords)) return false;
	boxCtr[handleNum] += 0.1f;
	if (!viz->projectPointToWin(boxCtr, dispCoords)) return false;
	//Direction vector is difference:
	_handleProjVec[0] = dispCoords[0] - winCoords[0];
	_handleProjVec[1] = dispCoords[1] - winCoords[1];
	float vecNorm = sqrt(_handleProjVec[0]*_handleProjVec[0]+_handleProjVec[1]*_handleProjVec[1]);
	if (vecNorm == 0.f) return false;
	_handleProjVec[0] /= vecNorm;
	_handleProjVec[1] /= vecNorm;
	return true;
}
// Project the current mouse coordinates to a line in screen space.
// The line starts at the mouseDownPosition, and points in the
// direction resulting from projecting to the screen the axis 
// associated with the dragHandle.  Returns false on error.
bool TranslateStretchManip::projectPointToLine(double mouseCoords[2], double projCoords[2]){
	//  State saved at a mouse press is:
	//	mouseDownPoint[2] = P
	//  _handleProjVec[2] unit vector (U)
	//
	// When the mouse is moved, project to the line:
	// point Q projects to P + aU, where a = (Q-P).U = dotprod
	double diff[2];
	if (!_mouseDownHere) return false;
	diff[0] = mouseCoords[0] - _mouseDownPoint[0];
	diff[1] = mouseCoords[1] - _mouseDownPoint[1];
	double dotprod = diff[0]*_handleProjVec[0]+diff[1]*_handleProjVec[1];
	projCoords[0] = _mouseDownPoint[0] + dotprod*_handleProjVec[0];
	projCoords[1] = _mouseDownPoint[1] + dotprod*_handleProjVec[1];
	
	return true;
}
//Find the handle extents using the boxExtents in world coords
//Set the octant to be 0 if the sortPosition is just the 
//handleNum
//If this is on the same axis as the selected handle it is displaced by _dragDistance
//

void TranslateStretchManip::
makeHandleExtents(int sortPosition, double handleExtents[6], int octant, double boxExtents[6]){
	//Identify the axis this handle is on:
	int axis = (sortPosition<3) ? (2-sortPosition) : (sortPosition-3);
	int newPosition = sortPosition;
	if ((octant>>axis)&1) newPosition = 5 - sortPosition;
	
	//Now create the cube associated with newPosition.  It's just the handle translated
	//up or down in the direction associated with newPosition
	float worldHandleDiameter = _handleSizeInScene;
	for (int coord = 0; coord<3; coord++){
		//Start at the box center position
		handleExtents[coord] = .5f*(-worldHandleDiameter +(boxExtents[coord+3] + boxExtents[coord]));
		handleExtents[coord+3] = .5f*(worldHandleDiameter +(boxExtents[coord+3] + boxExtents[coord]));
		
		if (coord == axis){//Translate up or down along this axis
			//The translation is 2 handles + .5 box thickness
			double boxWidth = (boxExtents[coord+3] - boxExtents[coord]);
			if (newPosition < 3){ //"low" handles are shifted down in the coord:
				handleExtents[coord] -= (boxWidth*0.5f + 2.f*worldHandleDiameter);
				handleExtents[coord+3] -= (boxWidth*0.5f + 2.f*worldHandleDiameter);
			} else {
				handleExtents[coord]+= (boxWidth*0.5f + 2.f*worldHandleDiameter);
				handleExtents[coord+3]+= (boxWidth*0.5f + 2.f*worldHandleDiameter);
			}
		}
	}
	return;
}

//Draw all the faces of a cube with specified extents.
//Currently just used for handles.
void TranslateStretchManip::drawCubeFaces(double* extents, bool isSelected){
	
	glLineWidth( 2.0 );
	glEnable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (isSelected) glColor4fv(_faceSelectionColor);
	else glColor4fv(_unselectedFaceColor);
	
	
//Do left (x=0)
	glBegin(GL_QUADS);
	glVertex3f(extents[0], extents[1], extents[2]);
	glVertex3f(extents[0], extents[1], extents[5]);
	glVertex3f(extents[0], extents[4], extents[5]);
	glVertex3f(extents[0], extents[4], extents[2]);
	glEnd();
	
//do right 
	if (isSelected) glColor4fv(_faceSelectionColor);
	else glColor4fv(_unselectedFaceColor);
	glBegin(GL_QUADS);
	glVertex3f(extents[3], extents[1], extents[2]);
	glVertex3f(extents[3], extents[1], extents[5]);
	glVertex3f(extents[3], extents[4], extents[5]);
	glVertex3f(extents[3], extents[4], extents[2]);
	glEnd();
	
	
//top
	if (isSelected) glColor4fv(_faceSelectionColor);
	else glColor4fv(_unselectedFaceColor);
	glBegin(GL_QUADS);
	glVertex3f(extents[0], extents[4], extents[2]);
	glVertex3f(extents[3], extents[4], extents[2]);
	glVertex3f(extents[3], extents[4], extents[5]);
	glVertex3f(extents[0], extents[4], extents[5]);
	glEnd();
	
//bottom
	if (isSelected) glColor4fv(_faceSelectionColor);
	else glColor4fv(_unselectedFaceColor);
	glBegin(GL_QUADS);
	glVertex3f(extents[0], extents[1], extents[2]);
	glVertex3f(extents[0], extents[1], extents[5]);
	glVertex3f(extents[3], extents[1], extents[5]);
	glVertex3f(extents[3], extents[1], extents[2]);
	glEnd();
	
	//back
	if (isSelected) glColor4fv(_faceSelectionColor);
	else glColor4fv(_unselectedFaceColor);
	glBegin(GL_QUADS);
	glVertex3f(extents[0], extents[1], extents[2]);
	glVertex3f(extents[3], extents[1], extents[2]);
	glVertex3f(extents[3], extents[4], extents[2]);
	glVertex3f(extents[0], extents[4], extents[2]);
	glEnd();
	
	//do the front:
	//
	if (isSelected) glColor4fv(_faceSelectionColor);
	else glColor4fv(_unselectedFaceColor);
	glBegin(GL_QUADS);
	glVertex3f(extents[0], extents[1], extents[5]);
	glVertex3f(extents[3], extents[1], extents[5]);
	glVertex3f(extents[3], extents[4], extents[5]);
	glVertex3f(extents[0], extents[4], extents[5]);
	glEnd();
	
			
	glColor4f(1,1,1,1);
	glDisable(GL_BLEND);
}



//Note: This is performed in world coordinates!
//Determine intersection (in world coords!) of ray with handle
bool TranslateStretchManip::
rayHandleIntersect(double ray[3], const std::vector<double>& cameraPos, int handleNum, int faceNum, double intersect[3]){

	double val;
	double handleExtents[6];
	double boxExtents[6];
#ifdef	DEAD
	int timestep = _vis->getActiveAnimationParams()->GetCurrentTimestep();
#endif
	_params->GetBox()->GetLocalExtents(boxExtents,timestep);
	
	makeHandleExtents(handleNum, handleExtents, 0, boxExtents);
	int coord;
	
	switch (faceNum){
		case(2): //back; z = zmin
			val = handleExtents[2];
			coord = 2;
			break;
		case(3): //front; z = zmax
			val = handleExtents[5];
			coord = 2;
			break;
		case(1): //bot; y = min
			val = handleExtents[1];
			coord = 1;
			break;
		case(4): //top; y = max
			val = handleExtents[4];
			coord = 1;
			break;
		case(0): //left; x = min
			val = handleExtents[0];
			coord = 0;
			break;
		case(5): //right; x = max
			val = handleExtents[3];
			coord = 0;
			break;
		default:
			return false;
	}
	if (ray[coord] == 0.0) return false;
	float param = (val - cameraPos[coord])/ray[coord];
	for (int i = 0; i<3; i++){
		intersect[i] = cameraPos[i]+param*ray[i];
	}
	return true;
}
	

//Renders handles and box
//If it is stretching, it only moves the one handle that is doing the stretching
void TranslateStretchManip::render(){
	if (!_vis || !_params) return;
	
	double extents[6];
	//Calculate the box extents, and the viewer position, in the unit cube,
	//Without any rotation applied:
#ifdef	DEAD
	int timestep = _vis->getActiveAnimationParams()->GetCurrentTimestep();
#endif
	_params->GetBox()->GetStretchedLocalExtents(extents, timestep);
	
	ViewpointParams* myViewpointParams = _vis->getActiveViewpointParams();
	vector<double> camPos = myViewpointParams->getCameraPosLocal();
#ifdef	DEAD
	vector<double> stretch = _dataStatus->getStretchFactors();
#else
	vector<double> stretch(3,1.0);
#endif
	for (int i=0; i<stretch.size(); i++) {
		camPos[i] *= stretch[i];
	}

	//Set the handleSize, in user coords.
	//May need to adjust for scene stretch
	_handleSizeInScene = _vis->getPixelSize()*(float)HANDLE_DIAMETER;
	
	//Color depends on which item selected. (reg color vs highlight color)
	//Selected item is rendered at current offset
	//This will issue gl calls to render 6 cubes and 3 lines through them (to the center of the 
	//specified region).  If one is selected that line (and both cubes) are given
	//the highlight color
	
	glPushAttrib(GL_CURRENT_BIT);
	//Now generate each handle and render it.  Order is not important
	double handleExtents[6];
	for (int handleNum = 0; handleNum < 6; handleNum++){
		makeHandleExtents(handleNum, handleExtents, 0/*octant*/, extents);
		if (_selectedHandle >= 0){
			int axis = (_selectedHandle < 3) ? (2-_selectedHandle) : (_selectedHandle -3);
			//displace handleExtents appropriately
			if (_isStretching){
				//modify the extents for the one grabbed handle
				if ( _selectedHandle == handleNum){ 
					handleExtents[axis] += _dragDistance;
					handleExtents[axis+3] += _dragDistance;
				} //and make the handles on the non-grabbed axes move half as far:
				else if (handleNum != (5-_selectedHandle)){
					handleExtents[axis] += 0.5f*_dragDistance;
					handleExtents[axis+3] += 0.5f*_dragDistance;
				}
			} else {
				handleExtents[axis] += _dragDistance;
				handleExtents[axis+3] += _dragDistance;
			}
		}
		drawCubeFaces(handleExtents, (handleNum == _selectedHandle));
		drawHandleConnector(handleNum, handleExtents, extents);
	}
	//Then render the full box, unhighlighted and displaced
	drawBoxFaces();
	glPopAttrib();
	
}
//Draw the main box, just rendering the lines.
//the highlightedFace is not the same as the selectedFace!!
//

void TranslateStretchManip::drawBoxFaces(){
	double corners[8][3];
#ifdef	DEAD
	int timestep = _vis->getActiveAnimationParams()->GetCurrentTimestep();
#endif
	_params->GetBox()->calcLocalBoxCorners(corners, 0.f, timestep);

#ifdef	DEAD
	vector<double> stretch = _dataStatus->getStretchFactors();
#else
	vector<double> stretch(3,1.0);
#endif
	
	//Now the corners need to be put into the unit cube, and displaced appropriately
	//Either displace just half the corners or do the opposite ones as well.
	for (int cor = 0; cor < 8; cor++){
		for (int i=0; i<stretch.size(); i++) {
			corners[cor][i] *= stretch[i];
		}

		if (_selectedHandle >= 0) {
			int axis = (_selectedHandle < 3) ? (2-_selectedHandle):(_selectedHandle-3);
			//The corners associated with a handle are as follows:
			//If the handle is on the low end, i.e. _selectedHandle < 3, then
			// for axis == 0, handles on corners  1,3,5,7 (cor&1 is 1)
			// for axis == 1, handles on corners  0,1,4,5 (cor&2 is 0)
			// for axis == 2, handles on corners  4,5,6,7 (cor&4 is 4) 
			//HMMM That's wrong.  The same expression works for x,y, and z
			// for axis == 0, handles on corners 0,2,4,6
			// for axis == 2, handles on corners 0,1,2,3
			//This fixes a bug (2/7/07)
			if (_isStretching) {
				if (((cor>>axis)&1) && _selectedHandle < 3) continue;
				if (!((cor>>axis)&1) && _selectedHandle >= 3) continue;
			}
			corners[cor][axis] += _dragDistance;
		}
	}
	
	
	//Now render the edges:
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glLineWidth( 2.0 );
	glColor3f(1.f,0.f,0.f);
	glBegin(GL_LINES);
	glVertex3dv(corners[0]);
	glVertex3dv(corners[1]);
	glVertex3dv(corners[0]);
	glVertex3dv(corners[2]);
	glVertex3dv(corners[0]);
	glVertex3dv(corners[4]);

	glVertex3dv(corners[1]);
	glVertex3dv(corners[3]);
	glVertex3dv(corners[1]);
	glVertex3dv(corners[5]);

	glVertex3dv(corners[2]);
	glVertex3dv(corners[3]);
	glVertex3dv(corners[2]);
	glVertex3dv(corners[6]);

	glVertex3dv(corners[3]);
	glVertex3dv(corners[7]);

	glVertex3dv(corners[4]);
	glVertex3dv(corners[5]);
	glVertex3dv(corners[4]);
	glVertex3dv(corners[6]);

	glVertex3dv(corners[5]);
	glVertex3dv(corners[7]);

	glVertex3dv(corners[6]);
	glVertex3dv(corners[7]);
	glEnd();
	
	
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	
}



void TranslateStretchManip::
mouseRelease(float /*screenCoords*/[2]){
	//Need to commit to latest drag position
	//Are we dragging?
#ifdef	DEAD
	int timestep = _vis->getActiveAnimationParams()->GetCurrentTimestep();
#endif
	if (_selectedHandle >= 0){
		double boxExts[6];
		int axis = (_selectedHandle <3) ? (2-_selectedHandle): (_selectedHandle-3);
		//Convert _dragDistance to world coords:
		float dist = _dragDistance;
		_params->GetBox()->GetStretchedLocalExtents(boxExts,timestep);
	
		//Check if we are stretching.  If so, only move coords associated with
		//handle:
		if (_isStretching){
			//boxMin gets changed for nearHandle, boxMax for farHandle
			if (_selectedHandle < 3)
				boxExts[axis]+=dist;
			else 
				boxExts[axis+3]+=dist;
		} else {
			boxExts[axis]+=dist;
			boxExts[axis+3]+=dist;
		}
		_params->GetBox()->SetStretchedLocalExtents(boxExts,_params,timestep);
		
	}
	_dragDistance = 0.f;
	_selectedHandle = -1;
}

//Note: This is performed in local (unstretched) world coordinates!
void TranslateStretchManip::
captureMouseDown(int handleNum, const std::vector<double>& camPos, double* dirVec, int buttonNum, double strHandleMid[3]){
	//Grab a probe handle
	_selectedHandle = handleNum;
	_dragDistance = 0.f;
#ifdef	DEAD
	vector<double> stretch = _dataStatus->getStretchFactors();
#else
	vector<double> stretch(3,1.0);
#endif

	//Calculate intersection of ray with specified plane in unstretched coords
	//The selection ray is the vector from the camera to the intersection point
	for (int i = 0; i<3; i++) _initialSelectionRay[i] = strHandleMid[i]/stretch[i] - camPos[i];
	
	if (buttonNum > 1) _isStretching = true; 
	else _isStretching = false;
	//Reset any active rotation
	_tempRotation = 0.f;
	_tempRotAxis = -1;
}

//Slide the handle based on mouse move from previous capture.  
//Requires new direction vector associated with current mouse position
//The new face position requires finding the planar displacement such that 
//the ray (in the scene) associated with the new mouse position is as near
//as possible to the line projected from the original mouse position in the
//direction of planar motion.
//Initially calc done in  WORLD coords,
//Converted to stretched world coords for bounds testing,
//then final displacement is in cube coords.
//If constrain is true, the slide will not go out of the full extents of the data.
//

void TranslateStretchManip::
slideHandle(int handleNum, double movedRay[3], bool constrain){
	double normalVector[3] = {0.f,0.f,0.f};
	double q[3], r[3], w[3];
	assert(handleNum >= 0);
	int coord = (handleNum < 3) ? (2-handleNum):(handleNum-3);
	
	normalVector[coord] = 1.f;
	//Calculate W:
	vcopy(movedRay, w);
	vnormal(w);
	double scaleFactor = 1.f/vdot(w,normalVector);
	//Calculate q:
	vmult(w, scaleFactor, q);
	vsub(q, normalVector, q);
	
	//Calculate R:
	scaleFactor *= vdot(_initialSelectionRay, normalVector);
	vmult(w, scaleFactor, r);
	vsub(r, _initialSelectionRay, r);

	double denom = vdot(q,q);
	_dragDistance = 0.f;
	//convert to stretched world coords.
#ifdef	DEAD
	vector<double> stretch = _dataStatus->getStretchFactors();
#else
	vector<double> stretch(3,1.0);
#endif
	if (denom != 0.){
		_dragDistance = -vdot(q,r)/denom;
		_dragDistance *= stretch[coord];
	}
	
	//Make sure the displacement is OK.  Not allowed to
	//slide box out of full domain box.
	//If stretching, not allowed to push face through opposite face.
	
	//Do this calculation in stretched world coords
	double boxExtents[6];
	const double* sizes;
#ifdef	DEAD
	sizes = _dataStatus->getFullStretchedSizes();
	int timestep = _vis->getActiveAnimationParams()->GetCurrentTimestep();
#endif
	_params->GetBox()->GetStretchedLocalExtents(boxExtents,timestep);
	
	if (_isStretching){ //don't push through opposite face ..
		//Depends on whether we are pushing the "low" or "high" handle
		//E.g., The low handle is limited by the low end of the extents, and the
		//big end of the box
		if (handleNum < 3 ){ 
			if(_dragDistance + boxExtents[coord] > boxExtents[coord+3]) {
				_dragDistance = boxExtents[coord+3] - boxExtents[coord];
			}
			if(constrain && (_dragDistance + boxExtents[coord] < 0.)) {
				_dragDistance =  -boxExtents[coord];
			}
		} else {//Moving "high" handle:
			if (_dragDistance + boxExtents[coord+3] < boxExtents[coord]) {
				_dragDistance = boxExtents[coord] - boxExtents[coord+3];
			}
			if (constrain&&(_dragDistance + boxExtents[coord+3] > sizes[coord])) {
				_dragDistance = sizes[coord]-boxExtents[coord+3];
			}
		}
	} else if (constrain){ //sliding, not stretching
		//Don't push the box out of the full region extents:
		
		if (_dragDistance + boxExtents[coord] < 0.) {
			_dragDistance = -boxExtents[coord];
		}
		if (_dragDistance + boxExtents[coord+3] > sizes[coord]) {
			_dragDistance = sizes[coord] - boxExtents[coord+3];
		}
		
	}

}

//Draw a line connecting the specified handle to the box center.
//Highlight both ends of the line to the selected handle
//Note that handleExtents have already been displaced.
//if _isStretching is true,
//the box center only moves half as far.
void TranslateStretchManip::drawHandleConnector(int handleNum, double* handleExtents, double* boxExtents){
	//Determine the side of the handle and the side of the box that is connected:
	int axis = (handleNum <3) ? (2-handleNum) : (handleNum-3) ;
	bool posSide = (handleNum > 2);
	double handleDisp[3], boxDisp[3];
	//Every handle gets a line from its inside face to the center of the box.
	for (int i = 0; i< 3; i++){
		//determine the box displacement, based on dimension:
		if ((i == (2-_selectedHandle)) || (i ==(_selectedHandle -3))){
			if (_isStretching) boxDisp[i] = 0.5f*_dragDistance;
			else boxDisp[i] = _dragDistance;
		}
		else boxDisp[i] = 0.f;
		//The line to the handle is displaced according to what axis the
		//handle is on (independent of the _dragDistance)
		//
		if (i==axis){
			if (posSide) {//displace to lower (in-) side of handle
				handleDisp[i] = -0.5f*(handleExtents[i+3] - handleExtents[i]);
			}
			else {//displace to upper (out-) side of handle
				handleDisp[i] = 0.5f*(handleExtents[i+3] - handleExtents[i]);
			}
		}
		else {//don't displace other coordinates
			handleDisp[i] = 0.f;
		}
		
	}
	glLineWidth( 2.0 );
	glEnable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT, GL_FILL);
	if ((handleNum == _selectedHandle) || (handleNum ==(5-_selectedHandle))) 
		glColor4fv(_faceSelectionColor);
	else glColor4fv(_unselectedFaceColor);
	glBegin(GL_LINES);
	glVertex3f(0.5f*(handleExtents[3]+handleExtents[0])+handleDisp[0],0.5f*(handleExtents[4]+handleExtents[1])+handleDisp[1],0.5f*(handleExtents[5]+handleExtents[2])+handleDisp[2]);
	glVertex3f(0.5f*(boxExtents[3]+boxExtents[0])+boxDisp[0],0.5f*(boxExtents[4]+boxExtents[1])+boxDisp[1],0.5f*(boxExtents[5]+boxExtents[2])+boxDisp[2]);
	glEnd();
	glDisable(GL_BLEND);
}



//Subclass constructor.  Allows rotated cube to be translated and stretched
//
TranslateRotateManip::TranslateRotateManip(Visualizer* w, Params* p) : TranslateStretchManip(w,p){
	
}
void TranslateRotateManip::drawBoxFaces(){
	double corners[8][3];
	Permuter* myPermuter = 0;
	if (_isStretching){
		const vector<double> angles = _params->GetBox()->GetAngles();
		myPermuter = new Permuter(angles[0],angles[1],angles[2]);
	}
		
	_params->GetBox()->calcLocalBoxCorners(corners, 0.f, -1, _tempRotation, _tempRotAxis);
	//Now the corners need to be put into the unit cube, and displaced appropriately
	
	//Either displace just half the corners (when stretching) or do the opposite ones as well.
#ifdef	DEAD
	vector<double> stretch = _dataStatus->getStretchFactors();
#else
	vector<double> stretch(3,1.0);
#endif
	for (int cor = 0; cor < 8; cor++){
		for (int i=0; i<stretch.size(); i++) {
			corners[cor][i] *= stretch[i];
		}
		if (_selectedHandle >= 0) {
			int axis = (_selectedHandle < 3) ? (2-_selectedHandle):(_selectedHandle-3);
			//The corners associated with a handle are as follows:
			//If the handle is on the low end, i.e. _selectedHandle < 3, then
			// for axis == 0, handles on corners  0,2,4,6 (cor&1 is 0)
			// for axis == 1, handles on corners  0,1,4,5 (cor&2 is 0)
			// for axis == 2, handles on corners  0,1,2,3 (cor&4 is 0) 
			
			if (_isStretching) {
				//Based on the angles (phi theta psi) the user is grabbing 
				//a rotated side of the cube. These vertices slide with the mouse.
				//rotate the selected handle by theta, phi, psi to find the side that corresponds to
				//the corners that need to be moved
				int newHandle; 
				if (_selectedHandle<3) newHandle = myPermuter->permute(_selectedHandle-3);
				else newHandle = myPermuter->permute(_selectedHandle - 2);
				if (newHandle < 0) newHandle +=3;
				else newHandle +=2;
				int axis2 = (newHandle < 3) ? (2-newHandle):(newHandle-3);
				
				if (((cor>>axis2)&1) && newHandle < 3) continue;
				if (!((cor>>axis2)&1) && newHandle >= 3) continue;
			
			}
			corners[cor][axis] += _dragDistance;
		}
	}
	if (myPermuter) delete myPermuter;
	//Then the faces need to be rendered
	//Use the same face ordering as was used for picking (mouseOver())
	
	//determine the corners of the textured plane.
	//the front corners are numbered 4 more than the back.
	//Average the front and back to get the middle:
	//
	double midCorners[4][3];
	for (int i = 0; i<4; i++){
		for(int j=0; j<3; j++){
			midCorners[i][j] = 0.5f*(corners[i][j]+corners[i+4][j]);
		}
	}
	//Now render the edges:
	
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);
	glLineWidth( 2.0 );
	glColor3f(1.f,0.f,0.f);
	glBegin(GL_LINES);
	glVertex3dv(corners[0]);
	glVertex3dv(corners[1]);
	glVertex3dv(corners[0]);
	glVertex3dv(corners[2]);
	glVertex3dv(corners[0]);
	glVertex3dv(corners[4]);

	glVertex3dv(corners[1]);
	glVertex3dv(corners[3]);
	glVertex3dv(corners[1]);
	glVertex3dv(corners[5]);

	glVertex3dv(corners[2]);
	glVertex3dv(corners[3]);
	glVertex3dv(corners[2]);
	glVertex3dv(corners[6]);

	glVertex3dv(corners[3]);
	glVertex3dv(corners[7]);

	glVertex3dv(corners[4]);
	glVertex3dv(corners[5]);
	glVertex3dv(corners[4]);
	glVertex3dv(corners[6]);

	glVertex3dv(corners[5]);
	glVertex3dv(corners[7]);

	glVertex3dv(corners[6]);
	glVertex3dv(corners[7]);

	
	//Now do the middle:
	glVertex3dv(midCorners[0]);
	glVertex3dv(midCorners[1]);

	glVertex3dv(midCorners[0]);
	glVertex3dv(midCorners[2]);

	glVertex3dv(midCorners[2]);
	glVertex3dv(midCorners[3]);

	glVertex3dv(midCorners[3]);
	glVertex3dv(midCorners[1]);
	glEnd();

	
	
	
	glFlush();
	//glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	
	
	
}

//Slide the handle based on mouse move from previous capture.  
//Requires new direction vector associated with current mouse position
//The new face position requires finding the planar displacement such that 
//the ray (in the scene) associated with the new mouse position is as near
//as possible to the line projected from the original mouse position in the
//direction of planar motion.
//Everything is done in WORLD coords.
//

void TranslateRotateManip::
slideHandle(int handleNum, double movedRay[3], bool constrain){
	double normalVector[3] = {0.f,0.f,0.f};
	double q[3], r[3], w[3];
	assert(handleNum >= 0);
	int coord = (handleNum < 3) ? (2-handleNum):(handleNum-3);
	
	normalVector[coord] = 1.f;
	//qWarning(" Moved Ray %f %f %f", movedRay[0],movedRay[1],movedRay[2]);
	//Calculate W:
	vcopy(movedRay, w);
	vnormal(w);
	float scaleFactor = 1.f/vdot(w,normalVector);
	//Calculate q:
	vmult(w, scaleFactor, q);
	vsub(q, normalVector, q);
	
	//Calculate R:
	scaleFactor *= vdot(_initialSelectionRay, normalVector);
	vmult(w, scaleFactor, r);
	vsub(r, _initialSelectionRay, r);

	float denom = vdot(q,q);
	_dragDistance = 0.f;
	//Convert the drag distance to stretched world coords
#ifdef	DEAD
	vector<double> stretch = _dataStatus->getStretchFactors();
#else
	vector<double> stretch(3,1.0);
#endif
	if (denom != 0.f){
		_dragDistance = -vdot(q,r)/denom;
		_dragDistance *= stretch[coord];
	}
	
	
	//Make sure the displacement is OK.  
	//Not allowed to
	//slide or stretch box center out of full domain box.
	//Do this calculation in stretched world coords
	double boxExtents[6];
	_params->GetBox()->GetStretchedLocalExtents(boxExtents,-1);
	
	if (_isStretching){ //don't push through opposite face ..
		//We really should constrain the stretch to lie inside domain, if constrain is true!
		_dragDistance = constrainStretch(_dragDistance);
	} else { //sliding, not stretching
		//with constraint: Don't slide the center out of the full domain:
		if (constrain) {
			const double* sizes;
#ifdef	DEAD
			sizes = _dataStatus->getFullStretchedSizes();
#endif
			float boxCenter = 0.5f*(boxExtents[coord]+boxExtents[coord+3]);
			if (_dragDistance + boxCenter < 0.) {
				_dragDistance = -boxCenter;
			}
			if (_dragDistance + boxCenter > sizes[coord]){
				_dragDistance = sizes[coord] - boxCenter;
			} 
		}
	}

	//now convert from stretched world to cube coords:
	//_dragDistance /= _dataStatus->getMaxStretchedSize();

	
}
//Following code should be invoked during OpenGL rendering.
//Assumes coords are already setup.
//This renders the box and the associated handles.
//Takes into account theta/phi rotations
//If a handle is selected, it is highlighted, and the box is slid in the handle
//direction according to the current translation.
//This rendering takes place in cube coords
//The extents argument give the full domain coordinate extents in the unit cube
void TranslateRotateManip::render(){
	if (!_vis || !_params) return;
	
	double extents[6];
	//Calculate the box extents, and the viewer position, in the unit cube,
	//With any rotation applied:
	
	_params->GetBox()->calcContainingStretchedBoxExtents(extents,true);
	

	//Set the handleSize, in user coords.
	//May need to adjust for scene stretch
	_handleSizeInScene = _vis->getPixelSize()*(float)HANDLE_DIAMETER;
	
	//Color depends on which item selected. (reg color vs highlight color)
	//Selected item is rendered at current offset
	//This will issue gl calls to render 6 cubes and 3 lines through them (to the center of the 
	//specified region).  If one is selected that line (and both cubes) are given
	//the highlight color
	
	glPushAttrib(GL_CURRENT_BIT);
	//Now generate each handle and render it.  Order is not important
	double handleExtents[6];
	for (int handleNum = 0; handleNum < 6; handleNum++){
		makeHandleExtents(handleNum, handleExtents, 0/*octant*/, extents);
		if (_selectedHandle >= 0){
			//displace handleExtents appropriately
			int axis = (_selectedHandle < 3) ? (2-_selectedHandle) : (_selectedHandle -3);
			//displace handleExtents appropriately
			if (_isStretching){
				//modify the extents for the one grabbed handle
				if ( _selectedHandle == handleNum){ 
					handleExtents[axis] += _dragDistance;
					handleExtents[axis+3] += _dragDistance;
				} //and make the handles on the non-grabbed axes move half as far:
				else if (handleNum != (5-_selectedHandle)){
					handleExtents[axis] += 0.5f*_dragDistance;
					handleExtents[axis+3] += 0.5f*_dragDistance;
				}
			} else {
				handleExtents[axis] += _dragDistance;
				handleExtents[axis+3] += _dragDistance;
			}
		}
		drawCubeFaces(handleExtents, (handleNum == _selectedHandle));
		drawHandleConnector(handleNum, handleExtents, extents);
	}
	//Then render the full box, unhighlighted and displaced
	drawBoxFaces();
	glPopAttrib();
	
}
void TranslateRotateManip::
mouseRelease(float /*screenCoords*/[2]){
	//Need to commit to latest drag position
	Permuter* myPermuter = 0;
	//Are we dragging?
	if (_selectedHandle >= 0){
		double boxExts[6];
		int axis = (_selectedHandle <3) ? (2-_selectedHandle): (_selectedHandle-3);
		//Convert _dragDistance to world coords:
		float dist = _dragDistance;
		_params->GetBox()->GetStretchedLocalExtents(boxExts,-1);
		
		//Check if we are stretching.  If so, need to decide what
		//coords are associated with handle.  Only those are to be
		//translated.
		if (_isStretching){
			const vector<double> angles = _params->GetBox()->GetAngles();
			myPermuter = new Permuter(angles[0],angles[1],angles[2]);
			//Based on the angles (phi and theta) the user is grabbing 
			//a rotated side of the cube. These vertices slide with the mouse.
			//rotate the selected handle by theta, phi to find the side that corresponds to
			//the corners that need to be moved
			int newHandle; //This corresponds to the side that was grabbed
			if (_selectedHandle<3) newHandle = myPermuter->permute(_selectedHandle-3);
			else newHandle = myPermuter->permute(_selectedHandle - 2);
			if (newHandle < 0) newHandle +=3;
			else newHandle +=2;
			//And axis2 is the axis of the grabbed side
			int axis2 = (newHandle < 3) ? (2-newHandle):(newHandle-3);
				
			//Along axis, need to move center by half of distance.
			//boxMin gets changed for nearHandle, boxMax for farHandle
			//Need to also stretch box, since the rotation was about the middle:
			//Note that if axis2 is axis then we only change the max or the min
			boxExts[axis] += 0.5f*dist;
			boxExts[axis+3] += 0.5f*dist;
			//We need to stretch the size along axis2, without changing the center;
			//However this stretch is affected by the relative stretch factors of 
			//axis2 and axis
#ifdef	DEAD
			vector<double> stretch = _dataStatus->getStretchFactors();
#else
			vector<double> stretch(3,1.0);
#endif
			float dist2 = dist*stretch[axis2]/stretch[axis];
			if (_selectedHandle < 3){
				boxExts[axis2] += 0.5f*dist2;
				boxExts[axis2+3] -= 0.5f*dist2;
			}
			else {
				boxExts[axis2+3] += 0.5f*dist2;
				boxExts[axis2] -= 0.5f*dist2;
			}
			
		} else {
			boxExts[axis]+=dist;
			boxExts[axis+3]+=dist;
		}
		_params->GetBox()->SetStretchedLocalExtents(boxExts,_params,-1);
		
	}
	_dragDistance = 0.f;
	_selectedHandle = -1;
}
//Determine the right-mouse drag constraint based on
//requiring that the resulting box will have all its min coords less than
//its max coords.
double TranslateRotateManip::constrainStretch(double currentDist){
	double dist;
#ifdef	DEAD
	double dist = currentDist/_dataStatus->getMaxStretchedSize();
#endif
	double boxExts[6];
	_params->GetBox()->GetStretchedLocalExtents(boxExts,-1);
	
	const vector<double> angles = _params->GetBox()->GetAngles();
	Permuter* myPermuter = new Permuter(angles[0],angles[1],angles[2]);
	//Based on the angles (phi and theta) the user is grabbing 
	//a rotated side of the cube. These vertices slide with the mouse.
	//rotate the selected handle by theta, phi to find the side that corresponds to
	//the corners that need to be moved
	int newHandle; //This corresponds to the side that was grabbed
	if (_selectedHandle<3) newHandle = myPermuter->permute(_selectedHandle-3);
	else newHandle = myPermuter->permute(_selectedHandle - 2);
	if (newHandle < 0) newHandle +=3;
	else newHandle +=2;
	// axis2 is the axis of the grabbed side before rotation
	int axis2 = (newHandle < 3) ? (2-newHandle):(newHandle-3);
	// axis 1 is the axis in the scene
	int axis1 = (_selectedHandle < 3) ? (2 - _selectedHandle):(_selectedHandle -3);
	//Don't drag the z-axis if it's planar:
	if (axis2 == 2){
		if (_params->GetBox()->IsPlanar()) {
			delete myPermuter;
			return 0.f;
		}
	}
#ifdef	DEAD
			vector<double> stretch = _dataStatus->getStretchFactors();
#else
			vector<double> stretch(3,1.0);
#endif
	float corrFactor = 0.0;
#ifdef	DEAD
	float corrFactor = _dataStatus->getMaxStretchedSize()*stretch[axis2]/stretch[axis1]; 
#endif
	
	if (_selectedHandle < 3){
			if (dist*corrFactor > (boxExts[axis2+3]-boxExts[axis2])) 
				dist = (boxExts[axis2+3]-boxExts[axis2])/corrFactor;
	}
	else {
			if (dist*corrFactor < (boxExts[axis2]-boxExts[axis2+3])) 
				dist = (boxExts[axis2]-boxExts[axis2+3])/corrFactor;
	}
	delete myPermuter;
	
	return (dist);
#ifdef	DEAD
	return (dist*_dataStatus->getMaxStretchedSize());
#endif
}
TranslateRotateManip::Permuter::Permuter(double theta, double phi, double psi){
	//Find the nearest multiple of 90 degrees > 0
	theta += 44.;
	phi += 44.;
	psi += 44.;
	while(theta < 0.f) theta += 360.;
	while(phi < 0.f) phi += 360.;
	while(psi < 0.f) psi += 360.;
	//Then convert to right angles between 0 and 3:
	_thetaRot = (int)(theta/90.);
	_thetaRot = _thetaRot%4;
	_phiRot = (int)(phi/90.);
	_phiRot = _phiRot%4;
	_psiRot = (int)(psi/90.);
	_psiRot = _psiRot%4;
}
int TranslateRotateManip::Permuter::permute(int i){
	//first do the theta permutation:
	switch (_thetaRot){
		case 1: // 1->-2, -2->-1, -1 -> 2, 2 ->1
			if (abs(i) == 1) i = -2*i;
			else if (abs(i) == 2) i = i/2;
			break;
		case 2: 
			if (abs(i) < 3) i = -i;
			break;
		case 3: // 1->2, 2->-1, -1 -> -2, -2 ->1
			if (abs(i) == 1) i = 2*i;
			else if (abs(i) == 2) i = -i/2;
			break;
		default:
			break;
	}
	
	//then do the phi permutation:
	switch (_phiRot){
		case 1: // 1->3, 3->-1, -1 -> -3, -3 ->1
			if (abs(i) == 1) i = 3*i;
			else if (abs(i) == 3) i = -i/3;
			break;
		case 2: 
			if (abs(i) != 2) i = -i;
			break;
		case 3: // 1->3, 3->-1, -1 -> -3, -3 ->1
			if (abs(i) == 1) i = -3*i;
			else if (abs(i) == 3) i = i/3;
			break;
		default:
			break;
	}
	//finally do the psi permutation:
	switch (_psiRot){
		case 1: // 1->-2, -2->-1, -1 -> 2, 2 ->1
			if (abs(i) == 1) i = -2*i;
			else if (abs(i) == 2) i = i/2;
			break;
		case 2: 
			if (abs(i) < 3) i = -i;
			break;
		case 3: // 1->2, 2->-1, -1 -> -2, -2 ->1
			if (abs(i) == 1) i = 2*i;
			else if (abs(i) == 2) i = -i/2;
			break;
		default:
			break;
	}
	return i;
}
 
