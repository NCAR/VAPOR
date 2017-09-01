//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		TrackBall.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		July 2004
//
//	Description:	Defines the Trackball class:
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
 */

#ifndef TRACKBALL_H
#define TRACKBALL_H

#ifdef Darwin
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <vector>
#include <vapor/common.h>

/* These vector and quaternion macros complement similar
 * routines.
 */

/* The Trackball package gives that nice 3D rotation interface.
 * A TrackBall class is needed for each rotated scene.
 */
class Trackball {
  public:
    Trackball();
    Trackball(float scale[3]);
    void TrackballSetMatrix();
    void TrackballFlip(int axis);
    void TrackballSpin();
    void TrackballStopSpinning();
    int TrackballSpinning();
    void TrackballSetPosition(double newx, double newy);
    void TrackballRotate(double newx, double newy);
    void TrackballPan(double newx, double newy);
    void TrackballZoom(double newx, double newy);
    void TrackballCopyTo(Trackball *dst);

    void TrackballReset();

    void GetCenter(double center[3]) const {
        for (int i = 0; i < 3; i++)
            center[i] = _center[i];
    }
    //Note:  button is 1,2,3 for left, middle, right
    void MouseOnTrackball(int eventType, int thisButton, int xcrd, int ycrd, unsigned width, unsigned height);

    // Initialize the trackball, provide viewer position, direction, upvector,
    // and the center of rotation (all in trackball coordinate space)
    //
    void setFromFrame(
        const std::vector<double> &posvec, const std::vector<double> &dirvec,
        const std::vector<double> &upvec, const std::vector<double> &centerRot,
        bool perspective);

    void setFromFrame(
        const double posvec[3], const double dirvec[3],
        const double upvec[3], const double centerRot[3],
        bool perspective) {
        std::vector<double> pos, dir, up, center;
        for (int i = 0; i < 3; i++) {
            pos.push_back(posvec[i]);
            dir.push_back(dirvec[i]);
            up.push_back(upvec[i]);
            center.push_back(centerRot[i]);
        }
        setFromFrame(pos, dir, up, center, perspective);
    }

    void SetScale(const double scale[3]) {
        _scale[0] = scale[0];
        _scale[1] = scale[1];
        _scale[2] = scale[2];
    }

  private:
    void setCenter(const std::vector<double> &newCenter) {
        _center[0] = newCenter[0];
        _center[1] = newCenter[1];
        _center[2] = newCenter[2];
    }

    double _qrot[4];
    double _qinc[4];
    double _trans[3];
    double _scale[3];
    double _center[3];
    double _ballsize;
    double _lastx, _lasty;

    bool _perspective;
};

#endif // TRACKBALL_H
