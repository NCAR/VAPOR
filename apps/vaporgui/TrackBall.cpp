//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		TrackBall.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		July 2004
//
//	Description:	Implements the Trackball class:   
//		This was implemented from Ken Purcell's TrackBall
//		methods.  Additional methods provided to set the TrackBall
//		based on a viewing frame
//
/* Copyright (C) 1992  AHPCRC, Univeristy of Minnesota
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program in a file named 'Copying'; if not, write to
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139.
 */

/* Author:
 *	Ken Chin-Purcell (ken@ahpcrc.umn.edu)
 *	Army High Performance Computing Research Center (AHPCRC)
 *	Univeristy of Minnesota
 *
 * This class was extracted from glutil.c  See glutil.cpp for the revision history */


/* Trackball interface for 3D rotation:
 *
 * The TrackBall is simualted as a globe rotating about its center.
 * Mouse movement is mapped to a spot on the globe, which when moved
 * effects a rotation.
 *
 * In addition, this Trackball also keeps track of translation,
 * in the form of panning and zooming.
 *
 * To use:
 *	At the start of the program, call Trackball constructor to
 *		create a Trackball object.  You will want a separate
 *		Trackball for each rotatable scene.
 *	On a Button or Motion event, call
 *		MouseOnTrackball
 *	When you want to draw, call
 *		TrackballSetMatrix to modify the matrix.
 *	Misc. functions:
 *		TrackballReset to come to home position
 *		TrackballFlip to flip about an axis
 *		TrackballSpin to keep rotating by last increment
 *		TrackballStopSpinning to set increment to zero
 *
 * To change how TrackBall feels:
 *	tball->_scale[0]	: modify translation (pan) in x direction
 *	tball->_scale[1]	: modify translation (pan) in y direction
 *	tball->_scale[2]	: modify translation (zoom) in z direction
 *	tball->_ballsize : a bigger ball gives finer rotations
 *			  (default = 0.65)
 */


#include <cmath>
#include <iostream>
#include <vapor/glutil.h>
#include "TrackBall.h"
using namespace VAPoR;
void Trackball::TrackballReset()
{
    /* Bring trackball to home position, no translation, zero rotation.
     */
    qzero(_qrot);
    qzero(_qinc);
    vzero(_trans);
    vset(_scale, 1000000.0, 1000000.0, 1000000.0);
	//Default center of rotation:
    _center[0] = 0.5f;
    _center[1] = 0.5f;
    _center[2] = 0.5f;
}


Trackball::Trackball(void)
{
    _ballsize = 0.65f;
    vset(_scale, 1.0, 1.0, 1.0);
    TrackballReset();
}

Trackball::Trackball(float scale[3])
{
    _ballsize = 0.65f;
    vset(_scale, scale[0], scale[1], scale[2]);
    TrackballReset();
}



void Trackball::TrackballSetMatrix()
//Note perspective must be set previously in setFromFrame.
{
    /* Modify the modelview matrix by the trackball 
     * rotation and translation.
     */

	
	// Ugh. Use OpenGL to do matrix manipulation, then retrieve and store
	// results in member variable
	//
    if (_perspective) {
		// glTranslated(_center[0], _center[1], _center[2]);
		double m1[16];
		makeTransMatrix(_center, m1);

	    // glTranslated(_trans[0],  _trans[1],  _trans[2]);
		double m2[16];
		makeTransMatrix(_trans, m2);
		

		//	glMultMatrixd(m);
		double 	m3[16];
	    qmatrix(_qrot, m3);

		// glTranslated(-_center[0], -_center[1], -_center[2]);
		double ncenter[3];
		vcopy(_center, ncenter);
		vscale(ncenter, -1.0);

		double 	m4[16];
		makeTransMatrix(ncenter, m4);

		mmult(m2, m1, _modelViewMatrix);
		mmult(m3, _modelViewMatrix, _modelViewMatrix);
		mmult(m4, _modelViewMatrix, _modelViewMatrix);
    }
    else {
	    double scale_factor = 1.0;

	    if (_trans[2] < 0.0) {
			scale_factor = 5.0 / (1 - _trans[2]);
	    }
	    else {
			scale_factor = 5.0 + _trans[2];
	    }

	    // glTranslated(_trans[0],  _trans[1],  _trans[2]);
		double m1[16];
		makeTransMatrix(_center, m1);

		double m2[16];
	    qmatrix(_qrot, m2);

	    // glMultMatrixd(m);
		
		double scale[3] = {scale_factor, scale_factor, scale_factor};

	    // glScaled(scale_factor, scale_factor, scale_factor);
		double m3[16];
		makeScaleMatrix(scale, m3);

		mmult(m2, m1, _modelViewMatrix);
		mmult(m3, _modelViewMatrix, _modelViewMatrix);

    }

}

#ifndef	M_SQRT1_2
#define M_SQRT1_2  0.707106781186547524401f
#endif
static double q90[3][4] = {
    { M_SQRT1_2, 0.0, 0.0, M_SQRT1_2 },
    { 0.0, M_SQRT1_2, 0.0, M_SQRT1_2 },
    { 0.0, 0.0,-M_SQRT1_2, M_SQRT1_2 },
};

void Trackball::TrackballFlip( int axis)
{
    /* Rotate by 90 deg about the given axis.
     */
    if (axis >= 0  &&  axis < 3)
	qmult(q90[axis], _qrot, _qrot);
}


void Trackball::TrackballSpin()
{
    /* Rotationaly spin the trackball by the current increment.
     * Use this to implement rotational glide.
     */
    qmult(_qinc, _qrot, _qrot);
}


void Trackball::TrackballStopSpinning()
{
    /* Cease any rotational glide by zeroing the increment.
     */
    qzero(_qinc);
}


int Trackball::TrackballSpinning()
{
    /* If the trackball is gliding then the increment's angle
     * will be non-zero, and cos(theta) != 1, hence q[3] != 1.
     */
    return (_qinc[3] != 1);
}


void Trackball::TrackballSetPosition(double newx, double newy)
{
    /* Call this when the user does a mouse down.  
     * Stop the trackball glide, then remember the mouse
     * down point (for a future rotate, pan or zoom).
     */
    TrackballStopSpinning();
    _lastx = newx;
    _lasty = newy;
}


void Trackball::TrackballRotate(double newx, double newy)
{
    CalcRotation(_qinc, newx, newy, _lastx, _lasty, 
		 _ballsize);
    TrackballSpin();

    _lastx = newx;	/* remember for next time */
    _lasty = newy;
}


void Trackball::TrackballPan( double newx, double newy)
{
    _trans[0] += (newx - _lastx) * _scale[0];
    _trans[1] += (newy - _lasty) * _scale[1];

    _lastx = newx;	/* remember for next time */
    _lasty = newy;
}


void Trackball::TrackballZoom(double newx, double newy)
{
    _trans[2] += (newy - _lasty) * _scale[2];

    _lastx = newx;	/* remember for next time */
    _lasty = newy;

}


void Trackball::TrackballCopyTo(Trackball *dst)
{
    /* Copy the current roation of the trackball
     */
    qcopy(_qrot, dst->_qrot);
}

/*
 * eventNum is 0, 1, or 2 for press, move, or release
 * thisButton is Qt:LeftButton, RightButton, or MidButton
 */

void Trackball::MouseOnTrackball(int eventNum, int thisButton, int xcrd, int ycrd, unsigned width, unsigned height)
{
    /* Alter a Trackball structure given  mouse event.
     *   This routine *assumes* button 1 rotates, button 2 pans,
     *   and button 3 zooms!
     *
     * event		: ButtonPress, MotionNotify or ButtonRelease
     * width, height	: of event window
     * tball		: trackball to modify
     */
    double	x, y;
	static int	button;
	//Ignore time: Qt doesn't provide time with events (may need to revisit this later????)
    //static Time	downTime;

    
    switch (eventNum) {

	  case 0: //ButtonPress:
		x = ScalePoint(xcrd, 0, width);
		y = ScalePoint(ycrd, height, -((long) height));
		TrackballSetPosition(x, y);
		//Remember the last button pressed:
		button = thisButton;
		//downTime = event->xbutton.time;
		break;
	
	  case 1://MotionNotify:
		x = ScalePoint(xcrd, 0, width);
		y = ScalePoint(ycrd, height, -((long) height));
		switch (button) {
			case 1 ://left button
				TrackballRotate( x, y);
				break;
			case 2 : //middle button
				TrackballPan( x, y);
				break;
			case 3 : //right button
				TrackballZoom( x, y);
				break;
			default:
				break;
		}
		//downTime = event->xmotion.time;
		break;
	
	  case 2: //ButtonRelease:
		button = 1;//left
	//	if (event->xbutton.time - downTime > 250)
		//??? if (event->xbutton.time - downTime > 100)
		//???	TrackballStopSpinning(tball);
		break;
    }
}


bool Trackball::ReconstructCamera( 
	double position[3], double upVec[3], double viewDir[3]
) const {

    double minv[16];

    int rc = minvert(_modelViewMatrix, minv);
    if (rc<0) return(false);

    vscale(minv+8, -1.0);

    for (int i = 0; i<3; i++) {
        position[i] = minv[12+i]; //position vector is minv[12..14]
        upVec[i] = minv[4+i]; //up vector is minv[4..6]
        viewDir[i] = minv[8+i]; //view direction is minv[8..10]
    }
    vnormal(upVec);
    vnormal(viewDir);

	return(true);

}

// Set the quaternion and translation from a viewer frame
// Also happens to construct modelview matrix, but we don't use its translation
void Trackball::setFromFrame(
	const std::vector<double>& posvec, const std::vector<double>& dirvec, 
	const std::vector<double>& upvec, const std::vector<double>& centerRot,
	bool persp
) {
	//First construct the rotation matrix:
	double mtrx1[16];
	double trnsMtrx[16];
	double mtrx[16];
	setCenter(centerRot);
	makeTransMatrix(centerRot, trnsMtrx);
	makeModelviewMatrixD(posvec, dirvec, upvec, mtrx);
	
	//Translate on both sides by translation
	//first on left,
	mmult(trnsMtrx, mtrx, mtrx1);
	//then on right by negative:
	trnsMtrx[12] = -trnsMtrx[12];
	trnsMtrx[13] = -trnsMtrx[13];
	trnsMtrx[14] = -trnsMtrx[14];
	mmult(mtrx1,trnsMtrx, mtrx);
	double qrotd[4];
	//convert rotation part to quaternion:
	rotmatrix2q(mtrx, qrotd);
	for (int i = 0; i<4; i++) _qrot[i] = (double)qrotd[i];
	//set the translation?
	//If parallel (ortho) transform, z used for translation
	_perspective = persp;
	//vcopy(posvec, trans);
	for (int i = 0; i<3; i++) _trans[i] = (double)mtrx[i+12];
	
}
