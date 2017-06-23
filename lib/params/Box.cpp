//************************************************************************
//									*
//		     Copyright (C)  2011				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Box.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		April 2011
//
//	Description:	Implements the Box class.
//		Used to control extents and orientation of 2D and 3D data regions.
//		Supports time-varying extents.
//

#include <cfloat>
#include <vector>
#include <cassert>
#include <vapor/glutil.h>
#include <vapor/Box.h>

using namespace std;
using namespace VAPoR;
using namespace Wasp;

const std::string Box::m_anglesTag = "Angles";
const std::string Box::m_extentsTag = "Extents";
const std::string Box::m_planarTag = "Planar";
const std::string Box::m_orientationTag = "Orientation";

//
// Register class with object factory!!!
//
static ParamsRegistrar<Box> registrar(Box::GetClassType());

Box::Box(
    ParamsBase::StateSave *ssave) : ParamsBase(ssave, Box::GetClassType()) {

    MyBase::SetDiagMsg("Box::Box() this=%p", this);

    // Initialize with default box:
    //
    SetPlanar(false);

    vector<double> minExt, maxExt;
    for (int i = 0; i < 3; i++)
        minExt.push_back(0.0);
    for (int i = 0; i < 3; i++)
        maxExt.push_back(1.0);
    SetExtents(minExt, maxExt);

    vector<double> angles;
    angles.push_back(0.);
    angles.push_back(0.);
    angles.push_back(0.);
    SetAngles(angles);

    SetOrientation(2);
}

Box::Box(
    ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node) {
}

Box::~Box() {
    MyBase::SetDiagMsg("Box::~Box() this=%p", this);
}

void Box::SetExtents(
    const vector<double> &minExt, const vector<double> &maxExt) {
    assert(minExt.size() == maxExt.size());

    vector<double> exts;
    for (int i = 0; i < minExt.size(); i++)
        exts.push_back(minExt[i]);
    for (int i = minExt.size(); i < 3; i++)
        exts.push_back(0.0);

    for (int i = 0; i < maxExt.size(); i++)
        exts.push_back(maxExt[i]);
    for (int i = maxExt.size(); i < 3; i++)
        exts.push_back(0.0);

    SetValueDoubleVec(m_extentsTag, "Set box extents", exts);
    //cout << "Box::SetExts: " << exts[0] << " " << exts[1] << " " << exts[2] << " " << exts[3] << " " << exts[4] << " " << exts[5] << endl;
}

void Box::GetExtents(
    vector<double> &minExt, vector<double> &maxExt) const {
    minExt.clear();
    maxExt.clear();

    vector<double> defaultv;
    for (int i = 0; i < 3; i++)
        defaultv.push_back(0.0);
    for (int i = 0; i < 3; i++)
        defaultv.push_back(1.0);

    // exts is guaranteed to have the same number of elements as
    // defaultv
    //
    vector<double> exts = GetValueDoubleVec(m_extentsTag, defaultv);
    //cout << "Box::GetExts: " << exts[0] << " " << exts[1] << " " << exts[2] << " " << exts[3] << " " << exts[4] << " " << exts[5] << endl;

    int n = IsPlanar() ? 2 : 3;
    for (int i = 0; i < n; i++)
        minExt.push_back(exts[i]);
    for (int i = 0; i < n; i++)
        maxExt.push_back(exts[i + 3]);
}

void Box::SetPlanar(bool value) {

    SetValueLong(Box::m_planarTag, "Set box planar value", (long)value);
}

#ifdef DEAD
int Box::GetStretchedLocalExtents(double extents[6], int timestep) {
    double exts[6];
    int rc = GetLocalExtents(exts, timestep);
    if (rc)
        return rc;
    vector<double> stretch = _dataStatus->getStretchFactors();
    for (int i = 0; i < 6; i++)
        extents[i] = exts[i] * stretch[i % 3];
    return 0;
}
#endif

#ifdef DEAD
int Box::SetStretchedLocalExtents(const double extents[6], int timestep) {
    vector<double> exts;
    vector<double> stretch = _dataStatus->getStretchFactors();
    for (int i = 0; i < 6; i++)
        exts.push_back((double)(extents[i] / stretch[i % 3]));
    SetLocalExtents(exts, timestep);
}
#endif

#ifdef DEAD
void Box::
    buildLocalCoordTransform(
        double transformMatrix[12], double extraThickness, int timestep,
        double rotation, int axis) const {

    double theta, phi, psi;
    if (rotation != 0.) {
        convertThetaPhiPsi(&theta, &phi, &psi, axis, rotation);
    } else {
        vector<double> angles = GetAngles();
        theta = angles[0];
        phi = angles[1];
        psi = angles[2];
    }

    double boxSize[3];
    double boxExts[6];
    GetLocalExtents(boxExts, timestep);

    for (int i = 0; i < 3; i++) {
        boxExts[i] -= extraThickness;
        boxExts[i + 3] += extraThickness;
        boxSize[i] = (boxExts[i + 3] - boxExts[i]);
    }

    //Get the 3x3 rotation matrix:
    double rotMatrix[9];
    getRotationMatrix(theta * M_PI / 180., phi * M_PI / 180., psi * M_PI / 180., rotMatrix);

    //then scale according to box:
    transformMatrix[0] = 0.5 * boxSize[0] * rotMatrix[0];
    transformMatrix[1] = 0.5 * boxSize[1] * rotMatrix[1];
    transformMatrix[2] = 0.5 * boxSize[2] * rotMatrix[2];
    //2nd row:
    transformMatrix[4] = 0.5 * boxSize[0] * rotMatrix[3];
    transformMatrix[5] = 0.5 * boxSize[1] * rotMatrix[4];
    transformMatrix[6] = 0.5 * boxSize[2] * rotMatrix[5];
    //3rd row:
    transformMatrix[8] = 0.5 * boxSize[0] * rotMatrix[6];
    transformMatrix[9] = 0.5 * boxSize[1] * rotMatrix[7];
    transformMatrix[10] = 0.5 * boxSize[2] * rotMatrix[8];
    //last column, i.e. translation:
    transformMatrix[3] = .5 * (boxExts[3] + boxExts[0]);
    transformMatrix[7] = .5 * (boxExts[4] + boxExts[1]);
    transformMatrix[11] = .5 * (boxExts[5] + boxExts[2]);
}

//Determine a new value of theta phi and psi when the probe is rotated around either the
//x-, y-, or z- axis.  axis is 0,1,or 2 1. rotation is in degrees.
//newTheta and newPhi are in degrees, with theta between -180 and 180, phi between 0 and 180
//and newPsi between -180 and 180
void Box::convertThetaPhiPsi(
    double *newTheta, double *newPhi, double *newPsi,
    int axis, double rotation) const {

    //First, get original rotation matrix R0(theta, phi, psi)
    double origMatrix[9], axisRotate[9], newMatrix[9];
    vector<double> angles = GetAngles();
    getRotationMatrix(angles[0] * M_PI / 180., angles[1] * M_PI / 180., angles[2] * M_PI / 180., origMatrix);
    //Second, get rotation matrix R1(axis,rotation)
    getAxisRotation(axis, rotation * M_PI / 180., axisRotate);
    //New rotation matrix is R1*R0
    mmult33(axisRotate, origMatrix, newMatrix);
    //Calculate newTheta, newPhi, newPsi from R1*R0
    getRotAngles(newTheta, newPhi, newPsi, newMatrix);
    //Convert back to degrees:
    (*newTheta) *= (180. / M_PI);
    (*newPhi) *= (180. / M_PI);
    (*newPsi) *= (180. / M_PI);
    return;
}

//Following calculates box corners in user space.  Does not use
//stretching.
void Box::
    calcLocalBoxCorners(
        double corners[8][3], float extraThickness, int timestep,
        double rotation, int axis) const {

    double transformMatrix[12];
    buildLocalCoordTransform(transformMatrix, extraThickness, timestep, rotation, axis);
    double boxCoord[3];
    //Return the corners of the box (in world space)
    //Go counter-clockwise around the back, then around the front
    //X increases fastest, then y then z;

    //Fatten box slightly, in case it is degenerate.  This will
    //prevent us from getting invalid face normals.

    boxCoord[0] = -1.f;
    boxCoord[1] = -1.f;
    boxCoord[2] = -1.f;
    vtransform(boxCoord, transformMatrix, corners[0]);
    boxCoord[0] = 1.f;
    vtransform(boxCoord, transformMatrix, corners[1]);
    boxCoord[1] = 1.f;
    vtransform(boxCoord, transformMatrix, corners[3]);
    boxCoord[0] = -1.f;
    vtransform(boxCoord, transformMatrix, corners[2]);
    boxCoord[1] = -1.f;
    boxCoord[2] = 1.f;
    vtransform(boxCoord, transformMatrix, corners[4]);
    boxCoord[0] = 1.f;
    vtransform(boxCoord, transformMatrix, corners[5]);
    boxCoord[1] = 1.f;
    vtransform(boxCoord, transformMatrix, corners[7]);
    boxCoord[0] = -1.f;
    vtransform(boxCoord, transformMatrix, corners[6]);
}

//Find the smallest stretched extents containing the rotated box
//Similar to above, using stretched extents
void Box::calcRotatedStretchedBoxExtents(
    vector<double> stretchFactors, double *bigBoxExtents) const {
    //Determine the smallest axis-aligned cube that contains the probe.  This is
    //obtained by mapping all 8 corners into the space.
    //It will not necessarily fit inside the unit cube.
    double corners[8][3];
    calcLocalBoxCorners(corners, 0.f, -1);

    double boxMin[3], boxMax[3];
    int crd, cor;

    //initialize extents, and variables that will be min,max
    for (crd = 0; crd < 3; crd++) {
        boxMin[crd] = DBL_MAX;
        boxMax[crd] = -DBL_MAX;
    }

    for (cor = 0; cor < 8; cor++) {
        //make sure the container includes it:
        for (crd = 0; crd < 3; crd++) {
            if (corners[cor][crd] < boxMin[crd])
                boxMin[crd] = corners[cor][crd];
            if (corners[cor][crd] > boxMax[crd])
                boxMax[crd] = corners[cor][crd];
        }
    }

    for (crd = 0; crd < 3; crd++) {
        bigBoxExtents[crd] = (boxMin[crd] * stretchFactors[crd]);
        bigBoxExtents[crd + 3] = (boxMax[crd] * stretchFactors[crd]);
    }
}

//Find the smallest extents containing the rotated box
void Box::calcRotatedBoxExtents(double *bigBoxExtents) const {

    //Determine the smallest axis-aligned cube that contains the probe.  This is
    //obtained by mapping all 8 corners into the space.
    //It will not necessarily fit inside the unit cube.
    double corners[8][3];
    calcLocalBoxCorners(corners, 0.f, -1);

    double boxMin[3], boxMax[3];
    int crd, cor;

    //initialize extents, and variables that will be min,max
    for (crd = 0; crd < 3; crd++) {
        boxMin[crd] = DBL_MAX;
        boxMax[crd] = -DBL_MAX;
    }

    for (cor = 0; cor < 8; cor++) {
        //make sure the container includes it:
        for (crd = 0; crd < 3; crd++) {
            if (corners[cor][crd] < boxMin[crd])
                boxMin[crd] = corners[cor][crd];
            if (corners[cor][crd] > boxMax[crd])
                boxMax[crd] = corners[cor][crd];
        }
    }
    //Now convert the min,max back into extents

    for (crd = 0; crd < 3; crd++) {
        bigBoxExtents[crd] = boxMin[crd];
        bigBoxExtents[crd + 3] = boxMax[crd];
    }
}

//Clip the probe to the specified box extents
//Return false (and make no change) if the probe rectangle does not have a positive (rectangular)
//intersection in the box.

bool Box::cropToBox(const double bxExts[6]) {
    //0.  Initially need a startPoint that is in the box and on the probe center plane.
    //1.  Check if probe center works.  If not call intersectRotatedBox() to get another startPoint (on middle plane) inside box.
    //2.  Using the new startPoint, construct x-direction line.  Find its first two intersections (+ and -) with box.  Reset the start point to be the
    //		middle of the resulting line segment.
    //3.  Construct the y-direction line from the new startPoint.  Again find first + and - intersection points.
    //4.  Take 4 diagonals of x- and y- direction lines, find first box intersection (or corner if box intersection is after corner.
    //5.  find largest rectangle inside four diagonal points.  Use this as the new probe.

    //Transform the four probe corners to local region
    double transformMatrix[12];
    double prCenter[3]; // original probe center in world coords
    double startPoint[3];
    double pmid[3] = {0., 0., 0.};
    double pxp[3] = {1., 0., 0.};
    double pyp[3] = {0., 1., 0.};
    double psize[2];
    double prbexts[4]; //probe extents relative to startPoint, in the 2 probe axis directions.
    double probeCoords[2] = {0., 0.};
    double pendx[3], pendy[3];

    double exts[6];
    GetLocalExtents(exts);

    double boxExts[6];
    //shrink box slightly, otherwise errors occur with highly stretched domains.
    for (int i = 0; i < 3; i++) {
        boxExts[i] = bxExts[i] + (bxExts[i + 3] - bxExts[i]) * 1.e-4;
        boxExts[i + 3] = bxExts[i + 3] - (bxExts[i + 3] - bxExts[i]) * 1.e-4;
    }
    buildLocalCoordTransform(transformMatrix, 0.f, -1);
    //initially set startPoint to probe center:
    vtransform(pmid, transformMatrix, startPoint);
    vcopy(startPoint, prCenter);
    //Determine probe size in world coords.
    vtransform(pxp, transformMatrix, pendx);
    vtransform(pyp, transformMatrix, pendy);
    vsub(pendx, startPoint, pendx);
    vsub(pendy, startPoint, pendy);
    psize[0] = vlength(pendx);
    psize[1] = vlength(pendy);
    prbexts[2] = vlength(pendx);
    prbexts[3] = vlength(pendy);
    prbexts[0] = -prbexts[2];
    prbexts[1] = -prbexts[3];

    //Get direction vectors for rotated probe
    double rotMatrix[9];
    const vector<double> &angles = GetAngles();
    getRotationMatrix((float)(angles[0] * M_PI / 180.), (float)(angles[1] * M_PI / 180.), (float)(angles[2] * M_PI / 180.), rotMatrix);
    //determine the probe x- and y- direction vectors
    double vecx[3] = {1., 0., 0.};
    double vecy[3] = {0., 1., 0.};
    //Direction vectors:
    double dir[4][3];
    //Intersection parameters
    double result[4][2];
    double edgeDist[4]; //distances from start point to probe edges

    //Construct 2 rays in x-axis directions
    vtransform3(vecx, rotMatrix, dir[0]);
    vtransform3(vecy, rotMatrix, dir[1]);
    vnormal(dir[0]);
    vnormal(dir[1]);

    //also negate:
    vmult(dir[0], -1.f, dir[2]);
    vmult(dir[1], -1.f, dir[3]);

    //Test:  is startPoint inside box?
    bool pointInBox = true;
    for (int i = 0; i < 3; i++) {
        if (startPoint[i] < boxExts[i]) {
            pointInBox = false;
            break;
        }
        if (startPoint[i] > boxExts[i + 3]) {
            pointInBox = false;
            break;
        }
    }
    if (!pointInBox) {
        pointInBox = intersectRotatedBox(boxExts, startPoint, probeCoords);
        if (!pointInBox)
            return false;
        //Modify prbexts to have probe exts relative to new value of startPoint
        //probeCoords values are along the dir[0] and dir[1] directions, with a value of +1 indicating the probe x-width
        //Thus the startPoint in world coords is
        // prCenter + psize[0]*probeCoords[0]*dir[0] + psize[1]*probeCoords[1]*dir[1]
        // and the probe extents are similarly translated:
        prbexts[0] = prbexts[0] - probeCoords[0] * psize[0];
        prbexts[2] = prbexts[2] - probeCoords[0] * psize[0];
        prbexts[1] = prbexts[1] - probeCoords[1] * psize[1];
        prbexts[3] = prbexts[3] - probeCoords[1] * psize[1];
    }

    //Shoot rays in axis directions from startPoint.

    //Intersect each line with the box, get the nearest intersections
    int numpts;
    for (int i = 0; i < 4; i += 2) {
        numpts = rayBoxIntersect(startPoint, dir[i], boxExts, result[i]);
        //Each ray should have two intersection points with the box
        if (numpts < 2 || result[i][1] < 0.0)
            return false;

        //find the distance from the start point to the second intersection point
        double interpt[3];
        //calculate the intersection point
        for (int j = 0; j < 3; j++) {
            interpt[j] = result[i][1] * dir[i][j] + startPoint[j];
        }
        //find the distances from the intersection points to starting point
        for (int j = 0; j < 3; j++) {
            interpt[j] = interpt[j] - startPoint[j];
        }
        edgeDist[i] = vlength(interpt);
        //shorten the distance if it exceeds the probe extent in that direction
        if (i == 0 && edgeDist[i] > prbexts[2])
            edgeDist[i] = prbexts[2];
        if (i == 2 && edgeDist[i] > -prbexts[0])
            edgeDist[i] = -prbexts[0];
    }
    //Find the midpoint of the line connecting the x intersections
    double midDist = edgeDist[0] - edgeDist[2];
    //Move startPoint  to the center
    for (int i = 0; i < 3; i++) {
        startPoint[i] = startPoint[i] + dir[0][i] * midDist * 0.5;
    }
    //Modify edgeDist so that the new startPoint is the center.
    edgeDist[0] = edgeDist[2] = 0.5 * (edgeDist[0] + edgeDist[2]);

    //Now shoot rays in the y directions
    for (int i = 1; i < 4; i += 2) {
        numpts = rayBoxIntersect(startPoint, dir[i], boxExts, result[i]);
        //Each ray should have two intersection points with the box
        if (numpts < 2 || result[i][1] < 0.0)
            return false;

        //find the distance from the start point to the second intersection point
        double interpt[3];
        //calculate the intersection point
        for (int j = 0; j < 3; j++) {
            interpt[j] = result[i][1] * dir[i][j] + startPoint[j];
        }
        //find the distances from the intersection points to starting point
        for (int j = 0; j < 3; j++) {
            interpt[j] = interpt[j] - startPoint[j];
        }
        edgeDist[i] = vlength(interpt);
        //Shorten the distance if it exceeds the probe
        if (i == 1 && edgeDist[i] > prbexts[3])
            edgeDist[i] = prbexts[3];
        if (i == 3 && edgeDist[i] > -prbexts[1])
            edgeDist[i] = -prbexts[1];
    }

    //Now shoot rays in the diagonal directions from startPoint.
    //First determine the diagonal directions and the distances to the diagonal corners
    double diagDirs[4][3];
    double diagDist[4];
    for (int j = 0; j < 4; j++) {
        //Determine vector from startPoint to corner:
        for (int i = 0; i < 3; i++) {
            diagDirs[j][i] = edgeDist[j] * dir[j][i] + edgeDist[(j + 1) % 4] * dir[(j + 1) % 4][i];
        }
        diagDist[j] = vlength(diagDirs[j]);
        vnormal(diagDirs[j]);
    }
    //Now shoot rays in the diagonal directions
    double diagInterDist[4];
    double diagInterPt[4][3];
    double component[4][2]; //components of the resulting diagonals along probe x and y axes
    for (int i = 0; i < 4; i++) {
        numpts = rayBoxIntersect(startPoint, diagDirs[i], bxExts, result[i]);
        //Each ray should have two intersection points with the box
        if (numpts < 2 || result[i][1] < 0.0)
            return false;
        //and result[i][1] is the distance along diagonal i to intersection 1

        //find the distance from the start point to the second intersection point
        //calculate the intersection point
        for (int j = 0; j < 3; j++) {
            diagInterPt[i][j] = result[i][1] * diagDirs[i][j] + startPoint[j];
        }

        //find the distances from the intersection points to starting point
        double interVec[3];
        for (int j = 0; j < 3; j++) {
            interVec[j] = diagInterPt[i][j] - startPoint[j];
        }
        diagInterDist[i] = vlength(interVec);
        //Make sure the diagonal distance does not exceed the distance to the corner
        double shrinkFactor = 1.;
        if (diagInterDist[i] > diagDist[i]) {

            shrinkFactor = diagDist[i] / diagInterDist[i];
        }
        //project in probe directions to get components:
        component[i][0] = vdot(dir[0], diagDirs[i]) * result[i][1] * shrinkFactor;
        component[i][1] = vdot(dir[1], diagDirs[i]) * result[i][1] * shrinkFactor;
    }

    //Find the x,y extents (relative to startPoint):
    double pExts[4];
    //maxx must be the smaller of the two x displacements:
    pExts[2] = Min(component[3][0], component[0][0]); //maxx
    pExts[0] = Max(component[1][0], component[2][0]); //minx
    pExts[1] = Max(component[2][1], component[3][1]); //miny
    pExts[3] = Min(component[0][1], component[1][1]); //maxy

    double wid = pExts[2] - pExts[0];
    double ht = pExts[3] - pExts[1];

    //New probe center is translated from startPoint by average of extents:
    //add dir[0]*(pexts[2]+pexts[0])*.5 to move probe x coordinate, similarly for y:
    //Use dir[] to hold the resulting displacements.
    double probeCenter[3];
    vcopy(startPoint, probeCenter);
    vmult(dir[0], 0.5 * (pExts[0] + pExts[2]), dir[0]);
    vmult(dir[1], 0.5 * (pExts[1] + pExts[3]), dir[1]);
    vadd(startPoint, dir[0], probeCenter);
    vadd(probeCenter, dir[1], probeCenter);

    double depth = exts[5] - exts[2];
    //apply these as offsets to startPoint, to get probe local extents.
    //Don't change the z extents.

    exts[0] = probeCenter[0] - wid * 0.5;
    exts[1] = probeCenter[1] - ht * 0.5;
    exts[3] = probeCenter[0] + wid * 0.5;
    exts[4] = probeCenter[1] + ht * 0.5;
    exts[2] = probeCenter[2] - depth * 0.5;
    exts[5] = probeCenter[2] + depth * 0.5;

    SetLocalExtents(exts);

    return true;
}

//Find a point that lies in the probe plane and in a box, if the probe intersects a face of the box.
//Return false if there is no such intersection
bool Box::intersectRotatedBox(double boxExts[6], double intersectPoint[3], double probeCoords[2]) {
    //Transform the four probe corners to local region
    double transformMatrix[12];
    double cor[4][3];               //probe corners in local user coords
    double pcorn[3] = {0., 0., 0.}; //local probe corner coordinates

    buildLocalCoordTransform(transformMatrix, 0.f, -1);

    bool cornerInFace[4][6];
    for (int i = 0; i < 4; i++) {
        //make pcorn rotate counter-clockwise around probe
        pcorn[0] = -1.;
        pcorn[1] = -1.;
        if (i > 1)
            pcorn[1] = 1.;
        if (i == 1 || i == 2)
            pcorn[0] = 1.;
        vtransform(pcorn, transformMatrix, cor[i]);
        for (int k = 0; k < 3; k++) {
            //Classify each corner as to whether it is inside or outside the half-space defined by each face
            //cornerInFace[i][j] is true if the cor[i][j] is inside the half-space
            if (cor[i][k] <= boxExts[k])
                cornerInFace[i][k] = false;
            else
                cornerInFace[i][k] = true;
            if (cor[i][k] >= boxExts[k + 3])
                cornerInFace[i][k + 3] = false;
            else
                cornerInFace[i][k + 3] = true;
        }
    }

    //initialize probe min & max:
    double minx = -1., miny = -1.;
    double maxx = 1., maxy = 1.;
    //2. For each box face:
    for (int face = 0; face < 6; face++) {
        int faceDim = face % 3; //(x, y, or z-oriented face)
        int faceDir = face / 3; //either low or high face
        //Intersect this face with four sides of probe.
        //A side of probe is determined by line (1-t)*cor[k] + t*cor[(k+1)%4], going from cor[k] to cor[k+1]
        //for each pair of corners, equation is
        // (1-t)*cor[k][faceDim] + t*cor[k+1][faceDim] = boxExts[faceDim+faceDir*3]
        // t*(cor[(k+1)%4][faceDim] - cor[k][faceDim]) = boxExts[faceDim+faceDir*3] - cor[k][faceDim]
        // i.e.: t = (boxExts[faceDim+faceDir*3] - cor[k][faceDim])/(cor[(k+1)%4][faceDim] - cor[k][faceDim]);
        int interNum = 0;
        double interPoint[2][3];
        double interT[2];
        int interSide[2];
        //a. determine either 2 or 0 intersection points between probe boundary and box face, by intersecting all sides of probe with face
        for (int k = 0; k < 4; k++) {
            double denom = (cor[(k + 1) % 4][faceDim] - cor[k][faceDim]);
            if (denom == 0.)
                continue;
            double t = (boxExts[faceDim + faceDir * 3] - cor[k][faceDim]) / denom;
            if (t < 0. || t > 1.)
                continue;
            for (int j = 0; j < 3; j++) {
                interPoint[interNum][j] = (1. - t) * cor[k][j] + t * cor[(k + 1) % 4][j];
            }
            //Replace t with T, going from -1 to +1, increasing with x and y
            //This simplifies the logic later.
            if (k > 1)
                t = 1. - t;                 //make t increase with x and y
            interT[interNum] = 2. * t - 1.; //make T go from -1 to 1 instead of 0 to 1
            interSide[interNum] = k;
            interNum++;
        }
        assert(interNum == 0 || interNum == 2);
        //Are there two intersections?
        if (interNum == 2) {
            //are they on opposite sides?
            if (interSide[1] - interSide[0] == 2) {
                //Does it intersect the two horizonal edges?
                if (interSide[0] == 0) {
                    //is vertex 0 in this half-space? If so use min t-coordinate to cut the probe max x-extents
                    if (cornerInFace[interSide[0]][face]) {
                        double mint = Min(interT[0], interT[1]);
                        if (maxx > mint)
                            maxx = mint;
                    } else { //must be vertex 0 is outside half-space, so use maxt to trim probe min x-extents
                        double maxt = Max(interT[0], interT[1]);
                        if (minx < maxt)
                            minx = maxt;
                    }
                } else { //It must intersect the two vertical edges, check if vertex 0 is inside half-space
                    assert(interSide[0] == 1);
                    if (cornerInFace[interSide[0]][face]) {
                        double mint = Min(interT[0], interT[1]);
                        if (maxy > mint)
                            maxy = mint;
                    } else { //must be vertex 0 is outside half-space, so use max to trim
                        double maxt = Max(interT[0], interT[1]);
                        if (miny < maxt)
                            miny = maxt;
                    }
                }
            } else { //The two intersections must cut off a corner of the probe
                //The possible cases for interSide's are: 0,1 (cuts of vertex 1); 0,3 (cuts off vertex 0);
                //1,2 (cuts off vertex 2); 2,3(cuts off vertex 3);  each case can exclude or include the corner vertex
                if (interSide[0] == 0 && interSide[1] == 1) {
                    //new corner is midpoint of line between the two intersection points.
                    double newcorx = 0.5 * (interT[0] + 1.);
                    double newcory = 0.5 * (interT[1] - 1.);
                    if (cornerInFace[interSide[1]][face]) {
                        //if vertex 1 is inside then minx must be at least as large as newcorx, maxy as small as newcory
                        minx = Max(minx, newcorx);
                        maxy = Min(maxy, newcory);
                    } else { //maxx must be as small as newcorx, miny must be as large as newcory
                        maxx = Min(maxx, newcorx);
                        miny = Max(miny, newcory);
                    }
                } else if (interSide[0] == 0 && interSide[1] == 3) {
                    //new corner is midpoint of line between the two intersection points.
                    double newcorx = 0.5 * (interT[0] - 1.);
                    double newcory = 0.5 * (interT[1] - 1.);
                    if (cornerInFace[interSide[0]][face]) {
                        //if vertex 0 is inside then maxx must be at least as small as newcorx, maxy as small as newcory
                        maxx = Min(maxx, newcorx);
                        maxy = Min(maxy, newcory);
                    } else { //minx must be as large as newcorx, miny must be as large as newcory
                        minx = Max(minx, newcorx);
                        miny = Max(miny, newcory);
                    }
                } else if (interSide[0] == 1 && interSide[1] == 2) {
                    //new corner is midpoint of line between the two intersection points.
                    double newcory = 0.5 * (interT[0] + 1.);
                    double newcorx = 0.5 * (interT[1] + 1.);
                    if (cornerInFace[interSide[1]][face]) {
                        //if vertex 2 is inside then minx must be at least as large as newcorx, miny as large as newcory
                        minx = Max(minx, newcorx);
                        miny = Max(miny, newcory);
                    } else { //maxx must be as small as newcorx, maxy must be as small as newcory
                        maxx = Min(maxx, newcorx);
                        maxy = Min(maxy, newcory);
                    }
                } else if (interSide[0] == 2 && interSide[1] == 3) {
                    //new corner is midpoint of line between the two intersection points.
                    double newcorx = 0.5 * (interT[0] - 1.);
                    double newcory = 0.5 * (interT[1] + 1.);
                    if (cornerInFace[interSide[1]][face]) {
                        //if vertex 3 is inside then maxx must be at least as small as newcorx, miny as large as newcory
                        maxx = Min(maxx, newcorx);
                        minx = Max(miny, newcory);
                    } else { //minx must be as large as newcorx, maxy must be as small as newcory
                        minx = Max(minx, newcorx);
                        maxy = Min(maxy, newcory);
                    }
                } else
                    assert(0);
            } //end of cutting off corner
        } else {
            //If no intersections, check if the probe is completely outside slab determined by the face
            //If any corner is outside, entire probe is outside, so check first corner

            if (faceDir == 0 && cor[0][faceDim] < boxExts[face])
                return false;
            if (faceDir == 1 && cor[0][faceDim] > boxExts[face])
                return false;

            //OK, entire probe is inside this face
        }
    } //finished with all 6 faces.
    //Use minx, miny, maxx, maxy to find a rectangle in probe and box

    //3.  rectangle defined as intersection of all limits found
    // probeCoords uses middle:
    probeCoords[0] = 0.5 * (minx + maxx);
    probeCoords[1] = 0.5 * (miny + maxy);
    // convert minx, miny, maxx, maxy to interpolate between 0 and 1 (instead of -1 and +1)
    maxx = 0.5 * (maxx + 1.);
    maxy = 0.5 * (maxy + 1.);
    minx = 0.5 * (minx + 1.);
    miny = 0.5 * (miny + 1.);
    //Determine center of intersection (in world coords) by bilinearly interpolating from corners using minx, maxx, miny,maxy,
    //then taking average.

    double newCor[4][3];
    for (int k = 0; k < 3; k++) {
        newCor[0][k] = ((1. - minx) * cor[0][k] + minx * cor[1][k]) * (1. - miny) +
                       miny * ((1. - minx) * cor[3][k] + minx * cor[2][k]);
        newCor[1][k] = ((1. - maxx) * cor[0][k] + maxx * cor[1][k]) * (1. - miny) +
                       miny * ((1. - maxx) * cor[3][k] + maxx * cor[2][k]);
        newCor[2][k] = ((1. - maxx) * cor[0][k] + maxx * cor[1][k]) * (1. - maxy) +
                       maxy * ((1. - maxx) * cor[3][k] + maxx * cor[2][k]);
        newCor[3][k] = ((1. - minx) * cor[0][k] + minx * cor[1][k]) * (1. - maxy) +
                       maxy * ((1. - minx) * cor[3][k] + minx * cor[2][k]);
        intersectPoint[k] = 0.25 * (newCor[0][k] + newCor[1][k] + newCor[2][k] + newCor[3][k]);
    }
    return true;
}

//Find probe extents that are maximal and fit in box
bool Box::fitToBox(const double boxExts[6]) {

    //Increase the box if it is flat:
    double modBoxExts[6];
    for (int i = 0; i < 3; i++) {
        modBoxExts[i] = boxExts[i];
        modBoxExts[i + 3] = boxExts[i + 3];
        if (boxExts[i] >= boxExts[i + 3]) {
            if (boxExts[i] > 0.f) {
                modBoxExts[i] = boxExts[i] * 0.99999f;
                modBoxExts[i + 3] = boxExts[i] * 1.00001f;
            } else if (boxExts[i] < 0.f) {
                modBoxExts[i] = boxExts[i] * 1.00001f;
                modBoxExts[i + 3] = boxExts[i] * 0.99999f;
            } else {
                modBoxExts[i] = -1.e-20f;
                modBoxExts[i + 3] = 1.e-20f;
            }
        }
    }

    //find a point in the probe that lies in the box.   Do this by finding the intersections
    //of the box with the probe plane and averaging the resulting points:
    double startPoint[3];
    double interceptPoints[6][3];
    int numintercept = interceptBox(modBoxExts, interceptPoints);
    if (numintercept < 3)
        return false;
    vzero(startPoint);
    for (int i = 0; i < numintercept; i++) {
        for (int j = 0; j < 3; j++) {
            startPoint[j] += interceptPoints[i][j] * (1.f / (double)numintercept);
        }
    }

    //Expand the probe so that it will exceed the box extents in all dimensions
    //Begin with the startPoint and intersect horizontal rays with the far edges of the box.

    double probeCenter[3];
    double exts[6];
    GetLocalExtents(exts);
    for (int i = 0; i < 3; i++)
        probeCenter[i] = 0.5 * (exts[i] + exts[i + 3]);

    double rotMatrix[9];
    const vector<double> &angles = GetAngles();
    getRotationMatrix((float)(angles[0] * M_PI / 180.), (float)(angles[1] * M_PI / 180.), (float)(angles[2] * M_PI / 180.), rotMatrix);

    //determine the probe x- and y- direction vectors
    double vecx[3] = {1., 0., 0.};
    double vecy[3] = {0., 1., 0.};

    //Direction vectors:
    double dir[4][3];
    //Intersection parameters
    double result[4][2];
    double edgeDist[4]; //distances from center to probe edges

    //Construct 4 rays in axis directions

    vtransform3(vecx, rotMatrix, dir[0]);
    vtransform3(vecy, rotMatrix, dir[1]);

    vnormal(dir[0]);
    vnormal(dir[1]);
    //also negate:
    vmult(dir[0], -1.f, dir[2]);
    vmult(dir[1], -1.f, dir[3]);

    //Intersect with each line
    int numpts;
    for (int i = 0; i < 4; i++) {
        numpts = rayBoxIntersect(startPoint, dir[i], modBoxExts, result[i]);
        //Each ray should have two intersection points with the box
        if (numpts < 2 || result[i][1] < 0.f)
            return false;
    }
    //Use the distance from the probe center to the second intersection point as the new probe size
    for (int i = 0; i < 4; i++) {
        double interpt[3];
        //calculate the intersection point
        for (int j = 0; j < 3; j++) {
            interpt[j] = result[i][1] * dir[i][j] + startPoint[j];
        }
        //find the distance from the intersection point to the probe center
        for (int j = 0; j < 3; j++) {
            interpt[j] = interpt[j] - probeCenter[j];
        }
        edgeDist[i] = vlength(interpt);
    }
    //Stretch a bit to ensure adequate coverage
    double wid = 2.1 * Max(edgeDist[0], edgeDist[2]);
    double ht = 2.1 * Max(edgeDist[1], edgeDist[3]);

    double depth = abs(exts[5] - exts[2]);
    exts[0] = probeCenter[0] - 0.5f * wid;
    exts[3] = probeCenter[0] + 0.5f * wid;
    exts[1] = probeCenter[1] - 0.5f * ht;
    exts[4] = probeCenter[1] + 0.5f * ht;
    exts[2] = probeCenter[2] - 0.5f * depth;
    exts[5] = probeCenter[2] + 0.5f * depth;

    SetLocalExtents(exts);
    bool success = cropToBox(boxExts);
    //bool success = true;
    return success;
}

//Calculate up to six intersections of box edges with probe plane, return the number found.
//Up to 6 intersection points are placed in intercept array
int Box::interceptBox(const double boxExts[6], double intercept[6][3]) {
    int numfound = 0;
    //Get the equation of the probe plane
    //First, find normal to plane:
    double rotMatrix[9];
    const double vecz[3] = {0., 0., 1.};
    const double vec0[3] = {0., 0., 0.};
    double probeNormal[3], probeCenter[3];
    const vector<double> &angles = GetAngles();
    getRotationMatrix((float)(angles[0] * M_PI / 180.), (float)(angles[1] * M_PI / 180.), (float)(angles[2] * M_PI / 180.), rotMatrix);

    vtransform3(vecz, rotMatrix, probeNormal);
    double transformMatrix[12];

    buildLocalCoordTransform(transformMatrix, 0.01, -1);
    vtransform(vec0, transformMatrix, probeCenter);
    vnormal(probeNormal);
    //The equation of the probe plane is dot(V, probeNormal) = dst:
    double dst = vdot(probeNormal, probeCenter);
    //Now intersect the plane with all 6 edges of box.
    //each edge is defined by two equations of the form
    // x = boxExts[0] or boxExts[3]; y = boxExts[1] or boxExts[4]; z = boxExts[2] or boxExts[5]
    for (int edge = 0; edge < 12; edge++) {
        //edge%3 is the coordinate that varies;
        //equation holds (edge+1)%3 to low or high value, based on (edge/3)%2
        //holds (edge+2) to low or high value based on (edge/6)
        //Thus equations associated with edge are:
        //vcoord = edge%3, coord1 is (edge+1)%3, coord2 is (edge+2)%3;
        // boxExts[vcoord] <= pt[vcoord] <= boxExts[vcoord+3]
        // pt[coord1] = boxExts[coord1+3*((edge/3)%2)]
        // pt[coord2] = boxExts[coord2+3*((edge/6))]
        int vcoord = edge % 3;
        int coord1 = (edge + 1) % 3;
        int coord2 = (edge + 2) % 3;
        double rhs = dst - boxExts[coord1 + 3 * ((edge / 3) % 2)] * probeNormal[coord1] - boxExts[coord2 + 3 * (edge / 6)] * probeNormal[coord2];
        // and the equation is V*probeNormal[vcoord] = rhs
        //Question is whether the other (vcoord) coordinate of the intersection point lies between
        //boxExts[vcoord] and boxExts[vcoord+3]

        if (probeNormal[vcoord] == 0.f)
            continue;
        if (rhs / probeNormal[vcoord] < boxExts[vcoord])
            continue;
        if (rhs / probeNormal[vcoord] > boxExts[vcoord + 3])
            continue;
        //Intersection found!
        intercept[numfound][coord1] = boxExts[coord1 + 3 * ((edge / 3) % 2)];
        intercept[numfound][coord2] = boxExts[coord2 + 3 * (edge / 6)];
        intercept[numfound][vcoord] = rhs / probeNormal[vcoord];
        numfound++;
        if (numfound == 6)
            return numfound;
    }
    return numfound;
}

void Box::
    getRotatedVoxelExtents(string varname, float voxdims[2], int numRefinements) {

    StructuredGrid *rGrid = GetDataMgr()->GetVariable(_dataStatus->getMinTimestep(), varname, numRefinements, 0);
    assert(rGrid);
    double exts[6], fullSizes[3];
    rGrid->GetUserExtents(exts);
    for (int i = 0; i < 3; i++)
        fullSizes[i] = exts[i + 3] - exts[i];
    double sliceCoord[3];
    //Can ignore depth, just mapping center plane
    sliceCoord[2] = 0.f;
    double transformMatrix[12];

    //Set up to transform from probe into volume:
    buildLocalCoordTransform(transformMatrix, 0.f, -1);

    //Get the data dimensions (at this resolution):
    size_t dataSize[3];

    //Start by initializing integer extents
    rGrid->GetDimensions(dataSize);

    double cor[4][3];

    for (int cornum = 0; cornum < 4; cornum++) {
        double dataCoord[3];
        // coords relative to (-1,1)
        sliceCoord[1] = -1.f + 2. * (double)(cornum / 2);
        sliceCoord[0] = -1.f + 2. * (double)(cornum % 2);
        //Then transform to values in data
        vtransform(sliceCoord, transformMatrix, dataCoord);
        //Then get array coords:
        for (int i = 0; i < 3; i++) {
            cor[cornum][i] = ((double)dataSize[i]) * (dataCoord[i]) / (fullSizes[i]);
        }
    }
    double vecWid[3], vecHt[3];

    vsub(cor[1], cor[0], vecWid);
    vsub(cor[3], cor[1], vecHt);
    voxdims[0] = vlength(vecWid);
    voxdims[1] = vlength(vecHt);
    return;
}

void Box::rotateAndRenormalize(int axis, double rotVal) {

    //Now finalize the rotation
    double newTheta, newPhi, newPsi;
    convertThetaPhiPsi(&newTheta, &newPhi, &newPsi, axis, rotVal);
    double angles[3];
    angles[0] = newTheta;
    angles[1] = newPhi;
    angles[2] = newPsi;
    SetAngles(angles);
    return;
}

void Box::setBoxToExtents(const double extents[6]) {

    //First try to fit to extents.  If we fail, then move to fit
    bool success = fitToBox(extents);
    if (success)
        return;

    //Move the box so that it is centered in the extents:
    double pExts[6];
    GetLocalExtents(pExts);

    for (int i = 0; i < 3; i++) {
        double psize = pExts[i + 3] - pExts[i];
        pExts[i] = 0.5 * (extents[i] + extents[i + 3] - psize);
        pExts[i + 3] = 0.5 * (extents[i] + extents[i + 3] + psize);
    }
    SetLocalExtents(pExts, -1);
    success = fitToBox(extents);

    assert(success);

    return;
}

#endif
