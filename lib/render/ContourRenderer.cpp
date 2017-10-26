//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		isolinerenderer.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		October 2006
//
//	Description:	Implementation of the isolinerenderer class
//

#include <sstream>
#include <string>

#include <vapor/glutil.h> // Must be included first!!!

#include <vapor/Renderer.h>
#include <vapor/ContourRenderer.h>
#include <vapor/Visualizer.h>
//#include <vapor/params.h>
#include <vapor/ContourParams.h>
//#include <vapor/AnimationParams.h>
#include <vapor/regionparams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/DataStatus.h>
#include <vapor/errorcodes.h>
#include <vapor/GetAppPath.h>
#include <vapor/ControlExecutive.h>
//#include "textRenderer.h"

using namespace VAPoR;

static RendererRegistrar<ContourRenderer> registrar(
    ContourRenderer::GetClassType(), ContourParams::GetClassType());

ContourRenderer::ContourRenderer(const ParamsMgr *pm,
                                 string winName,
                                 string dataSetName,
                                 string instName,
                                 DataMgr *dataMgr)
    : Renderer(pm,
               winName,
               dataSetName,
               ContourParams::GetClassType(),
               ContourRenderer::GetClassType(),
               instName,
               dataMgr) {

    _lineCache.clear();
    _edgeSeg.clear();
    _edgeEdge1.clear();
    _edgeEdge2.clear();
    _markerBit.clear();
    _componentLength.clear();
    _endEdge.clear();
    _numIsovalsCached = 0;
    _gridSize = 0;
    _objectNums.clear();
}

/*
  Release allocated resources
*/

ContourRenderer::~ContourRenderer() {
    //De-allocate cache

    //for (size_t ts = _dataStatus->getMinTimestep(); ts <= _dataStatus->getMaxTimestep(); ts++){
    for (size_t ts = 0; ts <= 0; ts++) {
        cout << "Fudging timestep info in ContourRenderer.  Fix me!" << endl;
        invalidateLineCache((int)ts);
    }
    _lineCache.clear();
    //TextObject::clearTextObjects(this);
}

// Perform the rendering
//

//int ContourRenderer::_paintGL(DataMgr* dataMgr)
int ContourRenderer::_paintGL() {
    SetDiagMsg("ContourRenderer::_paintGL()");

    if (!cacheIsValid(GetCurrentTimestep())) {
        if (buildLineCache(_dataMgr) != 0)
            return -1;
        updateCacheKey((int)GetCurrentTimestep());
    }

    //
    //Perform OpenGL rendering of line segments
    //
    int rc = performRendering(GetCurrentTimestep(), _dataMgr);

    return rc;
}

int ContourRenderer::performRendering(size_t timestep, DataMgr *dataMgr) {

    EnableClipToBox();

    ContourParams *cParams = (ContourParams *)GetActiveParams();

    //Set up lighting
    glDisable(GL_LIGHTING);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(cParams->GetLineThickness());

    //Need to convert the iso-box coordinates to user coordinates, then to unit box coords.
    double transformMatrix[12];

    //Determine if terrain mapping will be used
    string hgtVar = cParams->GetHeightVariableName();
    bool mapToTerrain = (!hgtVar.empty() && !cParams->VariablesAre3D());
    StructuredGrid *heightGrid;
    int ts = GetCurrentTimestep();
    float boxexts[6];
    double userExts[6];
    if (mapToTerrain) {
        // See bottom of file!
        cout << "map to terrain not implemented" << endl;
    }

    glBegin(GL_LINES);

    vector<double> minExt, maxExt;
    cParams->GetBox()->GetExtents(minExt, maxExt);
    double pointa[3], pointb[3]; //points in cache
    pointa[2] = pointb[2] = 0.;

    for (int iso = 0; iso < cParams->GetNumContours(); iso++) {
        float lineColor[3];
        cParams->GetLineColor(iso, lineColor);
        glColor3fv(lineColor);

        pair<int, int> mapPair = make_pair(timestep, iso);
        vector<float *> lines = _lineCache[mapPair];
        for (int linenum = 0; linenum < lines.size(); linenum++) {
            pointa[0] = lines[linenum][0];
            pointa[1] = lines[linenum][1];
            pointb[0] = lines[linenum][2];
            pointb[1] = lines[linenum][3];

            if (mapToTerrain) {
                cout << "map to terrain not implemented" << endl;
            }

            // Scale the cached points {-1:1} to the user extents
            double xSpan = maxExt[0] - minExt[0];
            pointa[0] = (1 + pointa[0]) / 2.f * xSpan + minExt[0];
            pointb[0] = (1 + pointb[0]) / 2.f * xSpan + minExt[0];

            double ySpan = maxExt[1] - minExt[1];
            pointa[1] = (1 + pointa[1]) / 2.f * ySpan + minExt[1];
            pointb[1] = (1 + pointb[1]) / 2.f * ySpan + minExt[1];

            double zSpan = maxExt[2] - minExt[2];
            pointa[2] = 500.f; //(1+pointa[2]) * zSpan;
            pointb[2] = 500.f; //(1+pointb[2]) * zSpan;

            glVertex3dv(pointa);
            glVertex3dv(pointb);
        }
    }
    glEnd();

    DisableClippingPlanes();

    return 0;
}
/*
  Set up the OpenGL rendering state, 
*/

int ContourRenderer::_initializeGL() {

    //  This does nothing...?
    //	_initialized = true;
    return 0;
}

int ContourRenderer::buildLineCache(DataMgr *dataMgr) {

    ContourParams *cParams = (ContourParams *)GetActiveParams();

    invalidateLineCache(GetCurrentTimestep());

    int ts = GetCurrentTimestep();
    string var = cParams->GetVariableName();
    int level = cParams->GetRefinementLevel();
    int lod = cParams->GetCompressionLevel();
    vector<double> varMin, varMax;
    dataMgr->GetVariableExtents(ts, var,
                                level, varMin, varMax);

    //StructuredGrid* varGrid;
    //StructuredGrid* hgtGrid;
    Grid *varGrid;
    Grid *hgtGrid;
    vector<string> varname;
    varname.push_back(var);
    bool is3D = cParams->VariablesAre3D();
    string hgtVar = cParams->GetHeightVariableName();

    varGrid = (StructuredGrid *)dataMgr->GetVariable(ts, var, level, lod);
    varGrid->SetInterpolationOrder(1);

    vector<double> boxMin, boxMax;
    cParams->GetBox()->GetExtents(boxMin, boxMax);

    //_gridSize = 1000;
    _gridSize = 25;
    float *dataVals = new float[_gridSize * _gridSize];

    //Set up to transform from isoline plane into volume:
    float a[2], b[2], constValue[2];
    int mapDims[3];

    // We are currently only supporting axis-orthogonal planes.  When we
    // support arbitrarily rotated planes, planeCoords[] will be a 3 element
    // array.
    //	double planeCoords[2];
    //vector<double> dataCoords(3);
    double dataCoords[3];

    double iIncrement = (boxMax[0] - boxMin[0]) / (float)_gridSize;
    double jIncrement = (boxMax[1] - boxMin[1]) / (float)_gridSize;

    vector<double> minu, maxu;
    varGrid->GetUserExtents(minu, maxu);

    boxMin[0] = minu[0];
    boxMax[0] = maxu[0];
    boxMin[1] = minu[1];
    boxMax[1] = maxu[1];

    float mv = varGrid->GetMissingValue();
    for (int i = 0; i < _gridSize; i++) {
        //		planeCoords[0] = -1. + 2.*(double)i/(_gridSize-1.);
        for (int j = 0; j < _gridSize; j++) {
            dataVals[i + j * _gridSize] = mv;

            //			int orientation = cParams->GetBox()->GetOrientation();
            dataCoords[0] = i * iIncrement + boxMin[0];
            dataCoords[1] = j * jIncrement + boxMin[1];
            dataCoords[2] = (varMax[2] - varMin[2]) / 2.f;

            //			planeCoords[1] = -1. + 2.*(double)j/(_gridSize-1.);
            //2D transform is a*x + b
            //			dataCoords[0] = a[0]*planeCoords[0] + b[0];
            //			dataCoords[1] = a[1]*planeCoords[1] + b[1];
            //			dataCoords[2] = 0.;
            //double val = varGrid->GetValue(dataCoords);
            double val = varGrid->GetValue(dataCoords[0], dataCoords[1], dataCoords[2]);
            dataVals[i + j * _gridSize] = val;
            //if (j/500==0)
            //cout << var << " " << dataCoords[0] << " " << dataCoords[1] << " " << dataCoords[2] << " " << val << endl;
            //find the coords that the texture maps to
            //			bool dataOK = true;
            //			for (int k = 0; k< 3; k++){
            //				if (dataCoords[k] < extExtents[k] || dataCoords[k] > extExtents[k+3]) dataOK = false;
            //				dataCoords[k] += minExts[k]; //Convert to user coordinates.
            //			}
            //			if(dataOK) { //find the coordinate in the data array
            //				dataVals[i+j*_gridSize] = isolineGrid[0]->GetValue(dataCoords[0],dataCoords[1],dataCoords[2]);
            //			}
            //cout << "coords " << dataCoords[0] << " " << dataCoords[1] << " " << dataCoords[2] << endl;
        }
    }
    //Unlock the StructuredGrid
    dataMgr->UnlockGrid(varGrid);
    //	if (varname.size()>1)dataMgr->UnlockGrid(isolineGrid);
    //Loop over each isovalue and cell, and classify the cell as to which edges are crossed by the isoline.
    //when there is an isoline crossing, a line segment is saved in the cache, defined by the two endpoints.
    const vector<double> &isovals = cParams->GetIsovalues(var);

    //Clear the textObjects (if they exist)
    //	TextObject::clearTextObjects(this);

    _objectNums.clear();
    for (int iso = 0; iso < isovals.size(); iso++) {
        buildEdges(iso, dataVals, mv);
    }

    _numIsovalsCached = isovals.size();

    delete[] dataVals;
    return 0;
}

//! Invalidate the cache for a specific time step.
void ContourRenderer::invalidateLineCache(int timestep) {
    int numisovals = numIsovalsInCache();
    for (int iso = 0; iso < numisovals; iso++) {
        pair<int, int> indexpair = make_pair(timestep, iso);
        for (int i = 0; i < _lineCache[indexpair].size(); i++) {
            delete _lineCache[indexpair][i];
        }
        _lineCache[indexpair].clear();
    }
    deleteCacheKey(timestep);
}

//! setupCache must be called whenever a new renderer is created
void ContourRenderer::setupCache() {

    ContourParams *cParams = (ContourParams *)GetActiveParams();
    //for (size_t ts = _dataStatus->getMinTimestep(); ts <= _dataStatus->getMaxTimestep(); ts++){
    for (size_t ts = GetCurrentTimestep(); ts <= GetCurrentTimestep(); ts++) {
        for (int iso = 0; iso < cParams->GetNumContours(); iso++) {
            pair<int, int> indexpair = make_pair((int)ts, iso);
            _lineCache[indexpair] = *(new vector<float *>);
        }
    }
    _numIsovalsCached = cParams->GetNumContours();
}
//Classify a cell to one of 9 possibilities:
//0: no crossing
//1,2,3,4 : cross at 1 corner (vertices 0,1,2,3 respectively)
//5,6: cross opposite edges (between vertices 0-1 & 2-3 for 5, and between  1-2 & 0-3 for 6.
//7,8: cross all 4 corners, center agrees or disagrees with vertex 0
//All but 7 & 8 come from a lookup of 8 inputs (4 vertices, each above or below isovalue)
//Note that none of the vertices should have missing value
int ContourRenderer::edgeCode(int i, int j, float isoval, float *dataVals) {
    // intersection code is 1 if it intersects the first edge,
    // 2 for the second edge, 4 for the third, 8 for the fourth.
    // resulting combinations include 1+2, 1+4, 1+8, 2+4, 2+8, 4+8, 1+2+4+8 = 3,5,6,9,10,12,15.
    // These remap as follows: 3->2; 5->5; 6->3; 9->1; 10->6; 12->4; 15-> 7 or 8
    int intersectionCode = 0;
    //check for crossing between (i,j) and (i+1,j)
    if ((dataVals[i + _gridSize * j] < isoval && dataVals[i + 1 + _gridSize * j] >= isoval) ||
        (dataVals[i + _gridSize * j] >= isoval && dataVals[i + 1 + _gridSize * j] < isoval))
        intersectionCode += 1;
    //check for crossing between (i+1,j+1) and (i+1,j)
    if ((dataVals[i + 1 + _gridSize * j] < isoval && dataVals[i + 1 + _gridSize * (j + 1)] >= isoval) ||
        (dataVals[i + 1 + _gridSize * j] >= isoval && dataVals[i + 1 + _gridSize * (j + 1)] < isoval))
        intersectionCode += 2;
    //check for crossing between (i,j+1) and (i+1,j+1)
    if ((dataVals[i + _gridSize * (j + 1)] < isoval && dataVals[i + 1 + _gridSize * (j + 1)] >= isoval) ||
        (dataVals[i + _gridSize * (j + 1)] >= isoval && dataVals[i + 1 + _gridSize * (j + 1)] < isoval))
        intersectionCode += 4;
    //check for crossing between (i,j+1) and (i,j)
    if ((dataVals[i + _gridSize * j] < isoval && dataVals[i + _gridSize * (j + 1)] >= isoval) ||
        (dataVals[i + _gridSize * j] >= isoval && dataVals[i + _gridSize * (j + 1)] < isoval))
        intersectionCode += 8;

    int ecode;
    float avgvalue;
    // Remap intersectionCode to ecode: 0->0, 3->2; 5->5; 6->3; 9->1; 10->6; 12->4; 15-> 7 or 8
    switch (intersectionCode) {
    case (0):
        ecode = 0;
        break;
    case (3):
        ecode = 2;
        break;
    case (5):
        ecode = 5;
        break;
    case (6):
        ecode = 3;
        break;
    case (9):
        ecode = 1;
        break;
    case (10):
        ecode = 6;
        break;
    case (12):
        ecode = 4;
        break;
    case (15): //disambiguate 7 and 8, based on whether or not average is on same side of isovalue as (i,j) vertex.
               //average is used as best approximation of value at center of cell.
        avgvalue = 0.25 * (dataVals[i + _gridSize * j] + dataVals[i + _gridSize * (j + 1)] + dataVals[i + 1 + _gridSize * (j + 1)] + dataVals[i + 1 + _gridSize * j]);
        if (((dataVals[i + _gridSize * j] < isoval) && (avgvalue < isoval)) ||
            ((dataVals[i + _gridSize * j] > isoval) && (avgvalue > isoval))) {
            //average agrees with (i,j), so use segments that connect 0-1 and 1-2 edge [case 2] as well as 2-3 and 3-0 [case 4]
            ecode = 7;
        } else //use segments that connect edges 1-2 and 2-3 as well as 3-0 and 0-1 [case 1 and case 3]
            ecode = 8;
        break;
    default:
        ecode = -1;
        assert(0);
    }
    return ecode;
}
int ContourRenderer::addLineSegment(int timestep, int isoIndex, float x1, float y1, float x2, float y2) {
    float *floatvec = new float[4];
    floatvec[0] = x1;
    floatvec[1] = y1;
    floatvec[2] = x2;
    floatvec[3] = y2;
    pair<int, int> indexpair = make_pair(timestep, isoIndex);
    _lineCache[indexpair].push_back(floatvec);
    return (_lineCache[indexpair].size() - 1);
}

//recall temporary mappings
//std::map< pair<int,int>, int> _edgeSeg; //map an edge to one segment
//std::map<pair<int,int>,pair<int,int> > _edgeEdge1; //map an edge to one adjacent edge;
//std::map<pair<int,int>,pair<int,int> > _edgeEdge2; //map an edge to other adjacent edge;
//std::map<pair<int,int>, bool> _markerBit;  //indicate whether or not an edge has been visited during traversal

//Whenever a segment is added, construct associated edge->edge mappings (both directions)
//and a mapping of the first edge to the segment.  The bidirectional edge mappings are used
//to enable traversal of the isoline in segment order.  The edge->segment mapping is used to
//determine the coordinates to place the annotation while traversing the isoline.
void ContourRenderer::addEdges(int segIndex, pair<int, int> edge1, pair<int, int> edge2) {

    std::map<pair<int, int>, pair<int, int>>::iterator edgeEdgeIter;

    //Set mapping of both edges to segment:
    _edgeSeg[edge1] = segIndex;
    _edgeSeg[edge2] = segIndex;
    //Set marker bits for both edges:
    _markerBit[edge1] = false;
    _markerBit[edge2] = false;
    //Create the edge1->edge2 mapping:
    //See if edge1->?? mapping is in first map:
    edgeEdgeIter = _edgeEdge1.find(edge1);
    if (edgeEdgeIter == _edgeEdge1.end()) {
        //Not found; insert it:
        _edgeEdge1[edge1] = edge2;
    } else {
        //check:  It should not be in the second map yet
        edgeEdgeIter = _edgeEdge2.find(edge1);
        assert(edgeEdgeIter == _edgeEdge2.end());
        _edgeEdge2[edge1] = edge2;
    }
    //Now create the edge2->edge1 mapping
    edgeEdgeIter = _edgeEdge1.find(edge2);
    if (edgeEdgeIter == _edgeEdge1.end()) {
        //Not found; insert it:
        _edgeEdge1[edge2] = edge1;
    } else {
        //check:  It should not be in the second map yet
        edgeEdgeIter = _edgeEdge2.find(edge2);
        assert(edgeEdgeIter == _edgeEdge2.end());
        _edgeEdge2[edge2] = edge1;
    }
}
//Use the edge-edge mappings to traverse each isoline
//First pass is simply to determine an endpoint and length.
//Second pass will write annotation
void ContourRenderer::traverseCurves(int iso) {
    //Initialize counters:
    int numComponents = 0;
    int maxLength = -1;
    int minLength = 100000000;
    int totLength = 0;
    int currentLength; //length of current component
    _componentLength.clear();
    _endEdge.clear();
    ContourParams *cParams = (ContourParams *)GetActiveParams();
    int timestep = (int)GetCurrentTimestep();
    //string isoText = std::to_string((long double)(cParams->GetIsovalues()[iso]));
    string isoText;
    string varName = cParams->GetVariableName();
    doubleToString((cParams->GetIsovalues(varName)[iso]), isoText, cParams->GetNumDigits());

    //Repeat the following until no more edges are found:
    //Find an unmarked edge.  Mark it.
    std::map<pair<int, int>, pair<int, int>>::iterator edgeEdgeIter;
    std::map<pair<int, int>, pair<int, int>>::iterator edgeEdgeTestIter;
    std::map<pair<int, int>, pair<int, int>>::iterator edgeEdgeTestIter2;

    //Iterate looking for unmarked edge in first mapping.  Note that all edges must appear in both mappings
    for (edgeEdgeIter = _edgeEdge1.begin(); edgeEdgeIter != _edgeEdge1.end(); edgeEdgeIter++) {
        pair<int, int> startingEdge = edgeEdgeIter->first;
        if (_markerBit[startingEdge])
            continue; //already marked; keep looking
        //OK, found an unmarked edge
        _markerBit[startingEdge] = true; //Mark it
        currentLength = 0;
        bool firstDirection = true;
        //Make sure there is a second direction possible
        edgeEdgeTestIter = _edgeEdge2.find(startingEdge);
        if (edgeEdgeTestIter == _edgeEdge2.end())
            firstDirection = false;

        pair<int, int> currentEdge = startingEdge;
        //Obtain next edges in both directions:
        //Repeat until cannot go further:
        while (1) {
            //Check an adjacent edge.  If it's not marked, make it the current edge, repeat
            //make sure current edge is in at least one mapping

            edgeEdgeTestIter = _edgeEdge1.find(currentEdge);
            if (edgeEdgeTestIter == _edgeEdge1.end()) {
                edgeEdgeTestIter = _edgeEdge2.find(currentEdge);
                if (edgeEdgeTestIter == _edgeEdge2.end()) {
                    //this means currentEdge goes nowhere (in either mapping)
                    //See if we can go the other direction from the start (note this code is replicated below...)
                    if (firstDirection) {
                        firstDirection = false;
                        //Try other direction at start
                        currentEdge = _edgeEdge2[startingEdge];
                        if (!_markerBit[currentEdge]) {
                            currentLength++;
                            _markerBit[currentEdge] = true;
                            continue; //continue our traversal with the new currentEdge
                        }
                    }
                    //Otherwise we are all done with this isoline; collect some stats
                    if (currentLength == 0)
                        break;
                    numComponents++;
                    totLength += currentLength;
                    if (currentLength > maxLength)
                        maxLength = currentLength;
                    if (currentLength < minLength)
                        minLength = currentLength;
                    _componentLength.push_back(currentLength);
                    _endEdge.push_back(currentEdge);
                    assert(_componentLength.size() == numComponents);
                    break; //exit while(1) loop
                }
            }
            //set nextEdge to the connected edge we found
            pair<int, int> nextEdge = edgeEdgeTestIter->second;
            if (!_markerBit[nextEdge]) {
                currentEdge = nextEdge;
                currentLength++;
                _markerBit[currentEdge] = true; //mark it...
                continue;
            } else {
                //it's marked false.  Need to consider other direction:
                edgeEdgeTestIter = _edgeEdge2.find(currentEdge);

                if ((edgeEdgeTestIter == _edgeEdge2.end()) || (_markerBit[edgeEdgeTestIter->second])) {
                    //Both ends are marked (or there is no other end) so
                    //we are at (one) end of isoline
                    //See if we can go the other direction from the start (this is replication of above code!)
                    if (firstDirection) {
                        firstDirection = false;
                        //Try other direction at start
                        currentEdge = _edgeEdge2[startingEdge];
                        if (!_markerBit[currentEdge]) {
                            _markerBit[currentEdge] = true;
                            currentLength++;
                            continue; //continue our traversal with the new currentEdge
                        }
                    }
                    //Otherwise we are all done with this isoline; collect some stats
                    if (currentLength == 0)
                        break;
                    numComponents++;
                    totLength += currentLength;
                    if (currentLength > maxLength)
                        maxLength = currentLength;
                    if (currentLength < minLength)
                        minLength = currentLength;
                    _componentLength.push_back(currentLength);
                    _endEdge.push_back(currentEdge);
                    assert(_componentLength.size() == numComponents);
                    break; //exit while(1) loop
                } else {
                    //Not marked; continue with nextEdge:
                    currentEdge = edgeEdgeTestIter->second;
                    currentLength++;
                    _markerBit[currentEdge] = true;
                    continue;
                }
            }
        }
    }
    //Done with traversals.  check if every edge has been marked
    /* Comment out these tests for the release version:
	for (edgeEdgeIter = _edgeEdge1.begin(); edgeEdgeIter != _edgeEdge1.end(); edgeEdgeIter++){
		pair<int,int> thisEdge = edgeEdgeIter->first;
		assert (_markerBit[thisEdge]) ;
	}
	for (edgeEdgeIter = _edgeEdge2.begin(); edgeEdgeIter != _edgeEdge2.end(); edgeEdgeIter++){
		pair<int,int> thisEdge = edgeEdgeIter->first;
		assert (_markerBit[thisEdge]) ;
	}
*/

#ifdef DEAD
    attachAnnotation(numComponents, iso);
#endif
}

#ifdef DEAD
void ContourRenderer::attachAnnotation(int numComponents, int iso) {
    std::map<pair<int, int>, pair<int, int>>::iterator edgeEdgeTestIter;
    double point1[3]; //point in local box
    double pointa[3]; //point in cache
    pointa[2] = 0.;
    int timestep = (int)GetCurrentTimestep();
    ContourParams *cParams = (ContourParams *)GetActiveParams();
    //Prepare to convert the iso-box coordinates to user coordinates, then to unit box coords.
    double transformMatrix[12];
    //	cParams->GetBox()->buildLocalCoordTransform(transformMatrix, 0.f, -1);
    //traverse each component, writing annotation at specified interval
    //Note that all marker bits are true,so we can use _markerBit[i]==false as a new marker
    //When textDensity is 1, there is annotation at every point.  When textDensity is 0.5 (typical)
    //there should be about A annotations in crossing the domain; i.e. annotation interval should be about 1/A times the grid size
    //when textDensity is .5, and A is a normalization constant.  So define
    // annotSpace = (2-g/A) + (g/A-1)/density where g is grid length or 2*A, whichever is larger
    //If the interval is shorter than the component and larger than 0.1 times the component, then just one annotation is generated
    float A = 3.;

    float g = (float)_gridSize;
    if (g < 2 * A)
        g = 2 * A;
    int annotSpace = (2 - g / A) + (g / A - 1.f) / cParams->GetTextDensity();
    int numAnnotations = 0;
    _markerBit.clear();
    for (int comp = 0; comp < numComponents; comp++) {
        int annotInterval = annotSpace;
        pair<int, int> currentEdge = _endEdge[comp];
        _markerBit[currentEdge] = true; //Mark it
        int length = _componentLength[comp];
        if (length < annotInterval / 10)
            continue; //No annotation for this component
        if (length < annotInterval)
            annotInterval = length;
        //Modify annotInterval so that it evenly divides length
        if (annotInterval < length) {
            int frac = length / annotInterval;
            annotInterval = length / frac;
        }
        //Determine the first point as halfway between 0 and annotInterval-1;
        assert(annotInterval > 0);
        int startDist = 1 + annotInterval / 2;
        //Traverse along component, marking as we go.
        //first, count until we get startDist along; after that, increment by annotInterval
        //Obtain next edges in both directions for traversal, but no need to go back to start.
        int advancedDist = 0;
        int currentAdvancedDist = 0;
        int gapInterval = startDist;
        vector<double> minExts, maxExts;
        _dataStatus->GetExtents(timestep, minExts, maxExts);
        std::pair<int, int> mapPair = make_pair(timestep, iso);
        vector<float *> lines = _lineCache[mapPair];
        while (1) {
            //Check an adjacent edge.  If it's not marked, make it the current edge, repeat
            //make sure current edge is in at least one mapping

            edgeEdgeTestIter = _edgeEdge1.find(currentEdge);
            if (edgeEdgeTestIter == _edgeEdge1.end()) {
                edgeEdgeTestIter = _edgeEdge2.find(currentEdge);
                if (edgeEdgeTestIter == _edgeEdge2.end()) {
                    //this means currentEdge goes nowhere (in either mapping); must be at end already
                    assert(advancedDist == length);
                    break; //exit while(1) loop
                }
            }
            //set nextEdge to the connected edge we found
            pair<int, int> nextEdge = edgeEdgeTestIter->second;
            if (!_markerBit[nextEdge]) { //not marked, so ok to continue
                currentEdge = nextEdge;
                advancedDist++;
                currentAdvancedDist++;
                _markerBit[currentEdge] = true; //mark it...
                if (currentAdvancedDist == gapInterval) {

                    //display annotation here!
                    int linenum = _edgeSeg[currentEdge];
                    pointa[0] = lines[linenum][0];
                    pointa[1] = lines[linenum][1];
                    vtransform(pointa, transformMatrix, point1);
                    //Convert local to user:
                    for (int i = 0; i < 3; i++)
                        point1[i] += minExts[i];

                    TextObject::addText(this, _objectNums[iso], point1);
                    numAnnotations++;
                    //Reset the next interval:
                    gapInterval = annotInterval;
                    currentAdvancedDist = 0;
                }
                continue;
            } else {
                //it's marked true.  Need to consider other direction:
                edgeEdgeTestIter = _edgeEdge2.find(currentEdge);

                if ((edgeEdgeTestIter == _edgeEdge2.end()) || (_markerBit[edgeEdgeTestIter->second])) {
                    //Both ends are marked (or there is no other end) so
                    //we are at end of isoline
                    assert(advancedDist == length);
                    break; //exit while(1) loop

                } else {
                    //Not marked; continue with nextEdge:
                    currentEdge = edgeEdgeTestIter->second;
                    advancedDist++;
                    currentAdvancedDist++;
                    _markerBit[currentEdge] = true; //mark it...
                    if (currentAdvancedDist == gapInterval) {

                        //display annotation here!
                        int linenum = _edgeSeg[currentEdge];
                        pointa[0] = lines[linenum][0];
                        pointa[1] = lines[linenum][1];
                        vtransform(pointa, transformMatrix, point1);
                        //Convert local to user:
                        for (int i = 0; i < 3; i++)
                            point1[i] += minExts[i];

                        TextObject::addText(this, _objectNums[iso], point1);
                        numAnnotations++;
                        //Reset the next interval:
                        gapInterval = annotInterval;
                        currentAdvancedDist = 0;
                    }
                    continue;
                }
            }
        }
        //Done with component
    }
}
#endif

void ContourRenderer::buildEdges(int iso, float *dataVals, float mv) {
    ContourParams *cParams = (ContourParams *)GetActiveParams();
    string varName = cParams->GetVariableName();
    const vector<double> &isovals = cParams->GetIsovalues(varName);
    if (cParams->GetTextDensity() > 0. && cParams->GetTextEnabled()) { //create a textObject to hold annotation of this isovalue
        //BLACK background!
        float bgc[4] = {0, 0, 0, 1.};

        float lineColor[4];
        cParams->GetLineColor(iso, lineColor);
        //		cParams->GetLineColor(lineColor);
        lineColor[3] = 1.;
        vector<string> vec;
        vec.push_back("fonts");
        vec.push_back("Vera.ttf");
        string isoText;
        doubleToString((cParams->GetIsovalues(varName)[iso]), isoText, cParams->GetNumDigits());
        //		int objNum = TextObject::addTextObject(this, GetAppPath("VAPOR","share",vec).c_str(),(int)cParams->GetTextSize(),lineColor, bgc,(int)cParams->GetTextLabelType(), isoText);
        //		_objectNums[iso] = objNum;
    }
    //Clear out temporary caches for this isovalue:

    _edgeSeg.clear();   //map an edge to one segment
    _edgeEdge1.clear(); //map an edge to one adjacent edge;
    _edgeEdge2.clear(); //map an edge to other adjacent edge;

    float isoval = (float)isovals[iso];
    int cellCase;
    float x1, y1, x2, y2; //coordinates of intersection points
    int segIndex;
    size_t timestep = GetCurrentTimestep();
    //loop over cells (identified by lower-left vertices
    for (int i = 0; i < _gridSize - 1; i++) {
        for (int j = 0; j < _gridSize - 1; j++) {
            //Determine which case is associated with cell cornered at i,j
            if (0) { //(dataVals[i+j*_gridSize] == mv) || (dataVals[i+1+j*_gridSize] == mv) ||
                //(dataVals[i+(j+1)*_gridSize] == mv) || (dataVals[i+1+(j+1)*_gridSize] == mv)) {
                cellCase = 0;
            } else
                cellCase = edgeCode(i, j, isoval, dataVals);

            //Note the vertices are numbered counterclockwise starting with 0 at (i,j)
            switch (cellCase) {
            case (0): //no lines
                break;
            case (1): //lines intersect 0-3 [point 1] and 0-1 [point 2]
            {
                y2 = -1. + 2. * (double)(j) / (_gridSize - 1.);
                x1 = -1. + 2. * (double)(i) / (_gridSize - 1.);

                x2 = interp_i(i, j, isoval, dataVals);
                y1 = interp_j(i, j, isoval, dataVals);
                //line segment connects horizontal edge i,j with vertical edge (i,j)
                pair<int, int> edge1 = make_pair(i, j);
                pair<int, int> edge2 = make_pair(-i - 1, -j - 1);
                segIndex = addLineSegment(timestep, iso, x1, y1, x2, y2);
                addEdges(segIndex, edge1, edge2);
            } break;
            case (2): //lines intersect between vertices 0-1 [1] and vertices 1-2 [2]
            {
                y1 = -1. + 2. * (double)j / (_gridSize - 1.);
                x2 = -1. + 2. * (double)(i + 1) / (_gridSize - 1.);
                x1 = interp_i(i, j, isoval, dataVals);
                y2 = interp_j(i + 1, j, isoval, dataVals);
                //line segment connects horizontal edge i,j with vertical edge (i+1,j)
                pair<int, int> edge1 = make_pair(i, j);
                pair<int, int> edge2 = make_pair(-i - 2, -j - 1);
                segIndex = addLineSegment(timestep, iso, x1, y1, x2, y2);
                addEdges(segIndex, edge1, edge2);
            } break;
            case (3): //lines intersect 1-2 [1] and 2-3 [2]
            {
                y2 = -1. + 2. * (double)(j + 1) / (_gridSize - 1.);
                x1 = -1. + 2. * (double)(i + 1) / (_gridSize - 1.);
                x2 = interp_i(i, j + 1, isoval, dataVals);
                y1 = interp_j(i + 1, j, isoval, dataVals);
                //line segment connects horizontal edge i,j+1 with vertical edge (i+1,j)
                pair<int, int> edge1 = make_pair(i, j + 1);
                pair<int, int> edge2 = make_pair(-i - 2, -j - 1);
                segIndex = addLineSegment(timestep, iso, x1, y1, x2, y2);
                addEdges(segIndex, edge1, edge2);
            } break;
            case (4): //lines intersect 2-3 [1] and 0-3 [2]
            {
                y1 = -1. + 2. * (double)(j + 1) / (_gridSize - 1.);
                x2 = -1. + 2. * (double)(i) / (_gridSize - 1.);
                x1 = interp_i(i, j + 1, isoval, dataVals);
                y2 = interp_j(i, j, isoval, dataVals);
                //line segment connects horizontal edge i,j+1 with vertical edge (i,j)
                pair<int, int> edge1 = make_pair(i, j + 1);
                pair<int, int> edge2 = make_pair(-i - 1, -j - 1);
                segIndex = addLineSegment(timestep, iso, x1, y1, x2, y2);
                addEdges(segIndex, edge1, edge2);
            } break;

            case (5): //lines intersect 0-1 [1] and 2-3 [2]
            {
                y2 = -1. + 2. * (double)(j + 1) / (_gridSize - 1.);
                y1 = -1. + 2. * (double)(j) / (_gridSize - 1.);
                x1 = interp_i(i, j, isoval, dataVals);
                x2 = interp_i(i, j + 1, isoval, dataVals);
                //line segment connects horizontal edge i,j with horizontal edge (i,j+1)
                pair<int, int> edge1 = make_pair(i, j);
                pair<int, int> edge2 = make_pair(i, j + 1);
                segIndex = addLineSegment(timestep, iso, x1, y1, x2, y2);
                addEdges(segIndex, edge1, edge2);
            } break;
            case (6): //line intersect 1-2 [1] and 0-3 [2]
            {
                x1 = -1. + 2. * (double)(i + 1) / (_gridSize - 1.);
                x2 = -1. + 2. * (double)(i) / (_gridSize - 1.);
                y1 = interp_j(i + 1, j, isoval, dataVals);
                y2 = interp_j(i, j, isoval, dataVals);
                //line segment connects vertical edge i+1,j with vertical edge (i,j)
                pair<int, int> edge1 = make_pair(-i - 2, -j - 1);
                pair<int, int> edge2 = make_pair(-i - 1, -j - 1);
                segIndex = addLineSegment(timestep, iso, x1, y1, x2, y2);
                addEdges(segIndex, edge1, edge2);
            } break;
            case (7): //both cases 2 and 4
                //lines intersect between vertices 0-1 [1] and vertices 1-2 [2]
                {
                    y1 = -1. + 2. * (double)j / (_gridSize - 1.);
                    x2 = -1. + 2. * (double)(i + 1) / (_gridSize - 1.);
                    x1 = interp_i(i, j, isoval, dataVals);
                    y2 = interp_j(i + 1, j, isoval, dataVals);
                    //line segment connects horizontal edge i,j with vertical edge (i+1,j)
                    pair<int, int> edge1 = make_pair(i, j);
                    pair<int, int> edge2 = make_pair(-i - 2, -j - 1);
                    segIndex = addLineSegment(timestep, iso, x1, y1, x2, y2);
                    addEdges(segIndex, edge1, edge2);
                    //lines intersect 2-3 [1] and 0-3 [2]
                    y1 = -1. + 2. * (double)(j + 1) / (_gridSize - 1.);
                    x2 = -1. + 2. * (double)(i) / (_gridSize - 1.);
                    x1 = interp_i(i, j + 1, isoval, dataVals);
                    y2 = interp_j(i, j, isoval, dataVals);
                    //line segment connects horizontal edge i,j+1 with vertical edge (i,j)
                    pair<int, int> edge3 = make_pair(i, j + 1);
                    pair<int, int> edge4 = make_pair(-i - 1, -j - 1);
                    segIndex = addLineSegment(timestep, iso, x1, y1, x2, y2);
                    addEdges(segIndex, edge3, edge4);
                }
                break;
            case (8): //both cases 1 and 3
                //lines intersect 0-3 [point 1] and 0-1 [point 2]
                {
                    y2 = -1. + 2. * (double)(j) / (_gridSize - 1.);
                    x1 = -1. + 2. * (double)(i) / (_gridSize - 1.);
                    x2 = interp_i(i, j, isoval, dataVals);
                    y1 = interp_j(i, j, isoval, dataVals);
                    //line segment connects horizontal edge i,j with vertical edge (i,j)
                    pair<int, int> edge1 = make_pair(i, j);
                    pair<int, int> edge2 = make_pair(-i - 1, -j - 1);
                    segIndex = addLineSegment(timestep, iso, x1, y1, x2, y2);
                    addEdges(segIndex, edge1, edge2);
                    //lines intersect 1-2 [1] and 2-3 [2]
                    y2 = -1. + 2. * (double)(j + 1) / (_gridSize - 1.);
                    x1 = -1. + 2. * (double)(i + 1) / (_gridSize - 1.);
                    x2 = interp_i(i, j + 1, isoval, dataVals);
                    y1 = interp_j(i + 1, j, isoval, dataVals);
                    //line segment connects horizontal edge i,j+1 with vertical edge (i+1,j)
                    pair<int, int> edge3 = make_pair(i, j + 1);
                    pair<int, int> edge4 = make_pair(-i - 2, -j - 1);
                    segIndex = addLineSegment(timestep, iso, x1, y1, x2, y2);
                    addEdges(segIndex, edge3, edge4);
                }
                break;
            default:
                assert(0);
            }
        } //for j
    }     //for i

    //Now traverse the edges to determine the isolines in order
    if (cParams->GetTextDensity() > 0. && cParams->GetTextEnabled()) {
        traverseCurves(iso);
    }

} // end buildEdges

bool ContourRenderer::cacheIsValid(int timestep) {
    //Check if the timestep is mapped:
    std::map<int, struct cacheKey *>::iterator it;
    it = cacheKeys.find(timestep);
    if (it == cacheKeys.end())
        return false;
    cacheKey *thisKey = it->second;
    ContourParams *cParams = (ContourParams *)GetActiveParams();
    if (thisKey->hgtVar != cParams->GetHeightVariableName())
        return false;
    if (thisKey->varname != cParams->GetVariableName())
        return false;
    if (thisKey->lod != cParams->GetCompressionLevel())
        return false;
    if (thisKey->refLevel != cParams->GetRefinementLevel())
        return false;
    if (thisKey->textDensity != cParams->GetTextDensity())
        return false;
    if (thisKey->textEnabled != cParams->GetTextEnabled())
        return false;
    string varName = cParams->GetVariableName();
    vector<double> ivals = cParams->GetIsovalues(varName);
    if (ivals.size() != thisKey->isovals.size())
        return false;
    for (int i = 0; i < ivals.size(); i++) {
        if (ivals[i] != thisKey->isovals[i])
            return false;
    }
    if (thisKey->is3D != cParams->VariablesAre3D())
        return false;
    //	Box* bx = cParams->GetBox();
    //	vector<double> exts = bx->GetLocalExtents();
    //	vector<double> angls = bx->GetAngles();
    //	for (int i = 0; i<3; i++){
    //		if (thisKey->extents[i] != exts[i]) return false;
    //		if (thisKey->extents[i+3] != exts[i+3]) return false;
    //		if (thisKey->angles[i] != angls[i]) return false;
    //	}
    return true;
}
void ContourRenderer::updateCacheKey(int timestep) {

    deleteCacheKey(timestep);
    cacheKey *thisKey = new cacheKey;
    ContourParams *cParams = (ContourParams *)GetActiveParams();
    thisKey->hgtVar = cParams->GetHeightVariableName();
    thisKey->varname = cParams->GetVariableName();
    thisKey->lod = cParams->GetCompressionLevel();
    thisKey->refLevel = cParams->GetRefinementLevel();
    thisKey->textDensity = cParams->GetTextDensity();
    thisKey->textEnabled = cParams->GetTextEnabled();
    string varName = cParams->GetVariableName();
    thisKey->isovals = cParams->GetIsovalues(varName);
    thisKey->is3D = cParams->VariablesAre3D();
    Box *bx = cParams->GetBox();
    thisKey->angles = bx->GetAngles();
    //	thisKey->extents = bx->GetLocalExtents();

    cacheKeys[timestep] = thisKey;
}

void ContourRenderer::deleteCacheKey(int timestep) {
    //Check if the timestep is mapped.  If so, delete cache key for that timestep
    std::map<int, struct cacheKey *>::iterator it;
    it = cacheKeys.find(timestep);
    if (it != cacheKeys.end()) { //Delete existing key
        if (it->second)
            delete it->second;
        cacheKeys.erase(it);
    }
}

/*
		//Note that the height variable may be retrieved when building the cache, however it will not actually
		//be used until render time, when it will be re-retrieved.
		vector<string> varname;
		varname.push_back(hgtVar);
		int level = cParams->GetRefinementLevel();
		int lod = cParams->GetCompressionLevel();
		
		vector<double> minExts, maxExts;
//		cParams->GetBox()->GetUserExtents(
//			minExts, maxExts, GetCurrentTimestep()
//		);
		dataMgr->GetVariableExtents(ts, hgtVar, 
			level, minExts, maxExts);

//		int rc = getGrids(
//			dataMgr, GetCurrentTimestep(), varname, minExts, maxExts,
//			&actualRefLevel, &lod, &heightGrid
//		);

		heightGrid = dataMgr->GetVariable(ts, hgtVar, level, lod);
		if(heightGrid == NULL){
			mapToTerrain = false;
			return -1;
		}
		heightGrid->GetUserExtents(minExts, maxExts);

*/

/*  Height stuff during isoline drawing loop:


				//Obtain the height corresponding to point1 and point2
				float z1 = heightGrid->GetValue(point1[0]+userExts[0],point1[1]+userExts[1],0.);
				float z2 = heightGrid->GetValue(point2[0]+userExts[0],point2[1]+userExts[1],0.);
				point1[2] = z1+boxexts[2]-userExts[2];
				point2[2] = z2+boxexts[2]-userExts[2];

*/
