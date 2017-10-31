//************************************************************************
//									*
//		     Copyright (C)  2014				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		contourrenderer.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		March 2014
//
//	Description:	Definition of the ContourRenderer class
//
#ifndef CONTOURRENDERER_H
#define CONTOURRENDERER_H

#include <GL/glew.h>
#ifdef Darwin
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <cassert>

#include <vapor/Renderer.h>
#include <vapor/ContourParams.h>

namespace VAPoR {

class DataMgr;
//! \class ContourRenderer
//! \brief Class that draws the contours (contours) as specified by IsolineParams
//! \author Alan Norton
//! \version 3.0
//! \date March 2014
class RENDER_API ContourRenderer : public Renderer {

  public:
    ContourRenderer(const ParamsMgr *pm,
                    string winName,
                    string dataSetName,
                    string instName,
                    DataMgr *dataMgr);

    virtual ~ContourRenderer();

    static string GetClassType() {
        return ("Contour");
    }

    //std::map<pair<int,int>,vector<float*> > _lineCache;
    //! Static method to create a renderer given the associated Params instance and visualizer
    //! \param[in] Visualizer* pointer to the visualizer where this will draw
    //! \param[in] RenderParams* pointer to the IsolineParams associated with this renderer
    //! \return ContourRenderer* new ContourRenderer instance.
#ifdef DEAD
    static Renderer *CreateInstance(
        Visualizer *v, RenderParams *rp, ShaderMgr *sm) {
        return new ContourRenderer(v, rp, sm);
    }
#endif

    //! \copydoc Renderer::_initializeGL()
    virtual int _initializeGL();
    //! \copydoc Renderer::_paintGL()
    //    virtual int		_paintGL(DataMgr* dmgr);
    virtual int _paintGL();

    //! Obtain the current line cache.  Each timestep and isovalue is
    //! associated with a vector of triples of floats.
    //! \return current lineCache
    const std::map<pair<int, int>, vector<float *>> &GetLineCache() { return _lineCache; }
    //! Check if the current cache is valid
    //! \param[in] timestep time step to be checked.
    //! \return true if cache is valid
    bool cacheIsValid(int timestep);

  private:
    int GetCurrentTimestep() { return 0; }

    //! Invalidate the cache of contours at a single time step
    //! \param[in] timestep is the Time Step to be invalidated.
    void invalidateLineCache(int timestep);

    //! Rebuild the entire line cache at the current time step
    //! \param[in] dmgr Current DataMgr
    int buildLineCache(DataMgr *dmgr);
    //! Reset the contour cache, prior to the next render that needs to build the cache.
    void setupCache();

    //! Render all the contour lines at a time step
    //! \param[in] timestep Time Step
    //! \param[in] dataMgr Current data manager.
    //! \return -1 if error occurred.
    int performRendering(size_t timestep, DataMgr *dataMgr);

    //! Determine the code associated with a grid cell based on its contour crossings
    //! \param[in] i grid cell x-index
    //! \param[in] j grid cell y-index
    //! \param[in] isoval isovalue being checked
    //! \param[in] dataVals Array of variable data values at grid vertices
    //! \return edge-code as follows:
    //! 0: no crossing
    //! 1,2,3,4 : cross at 1 corner (vertices 0,1,2,3 respectively)
    //! 5,6: cross opposite edges (between vertices 0-1 & 2-3 for 5, and between  1-2 & 0-3 for 6.)
    //! 7,8: cross all 4 corners, center agrees or disagrees with vertex 0
    int edgeCode(int i, int j, float isoval, float *dataVals);

    //! Insert a line segment into the set of segments associated with an contour
    //! \param[in] timestep Time Step
    //! \param[in] isoIndex Index of isovalue being followed
    //! \param[in] x1 x coordinate of first segment endpoint
    //! \param[in] y1 y coordinate of first segment endpoint
    //! \param[in] x2 x coordinate of second segment endpoint
    //! \param[in] y2 y coordinate of second segment endpoint
    //! \return index in the associated vector that was added.
    int addLineSegment(int timestep, int isoIndex, float x1, float y1, float x2, float y2);

    //! Count the number of isovalues in the current cache
    //! \return number of isovalues cached.
    int numIsovalsInCache() { return _numIsovalsCached; }

    //! Add an edge to the edge-edge mappings needed to traverse the contours, needed in order to place text annotation
    //! \param[in] segIndex Index associated with the edge being added
    //! \param[in] edge1 is (i,j) pair indicating the first edge
    //! \param[in] edge2 is (i,j) pair indicating the second edge
    void addEdges(int segIndex, pair<int, int> edge1, pair<int, int> edge2);

    //! Traverse the contour curves defined by the current edge->edge mappings, identifying the points where text annotation should be attached.
    //! \param[in] iso Index of isovalue associated with the curve
    void traverseCurves(int iso);

    //! Obtain the line segments associated with an isovalue at a timestep
    //! \param[in] timestep time step
    //! \param[in] isoindex index of isovalue
    //! \return current vector of points associated with the contours
    vector<float *> &getLineSegments(int timestep, int isoindex) {
        pair<int, int> indexpair = make_pair(timestep, isoindex);
        return _lineCache[indexpair];
    }

    //! Interpolate to find y coordinate of isovalue on the grid line from (i,j) to (i,j+1)
    //! \param[in] i x-coordinate of grid corner
    //! \param[in] j y-coordinate of grid corner
    //! \param[in] isoval iso-value to be interpolated along the line segment
    //! \param[in] dataVals array of floating point values of variable at all grid corners
    //! \return interpolated isovalue
    float interp_j(int i, int j, float isoval, float *dataVals) {
        return (-1. + 2. * (double)(j) / ((double)_sampleSize - 1.)                                                                        //y coordinate at (i,j)
                + 2. / (double)(_sampleSize - 1.)                                                                                          //y grid spacing
                      * (isoval - dataVals[i + _sampleSize * j]) / (dataVals[i + _sampleSize * (j + 1)] - dataVals[i + _sampleSize * j])); //ratio: 1 is iso at top, 0 if iso at bottom
    }

    //! Interpolate to find x coordinate of isovalue on the grid line from (i,j) to (i+1,j)
    //! \param[in] i x-coordinate of grid corner
    //! \param[in] j y-coordinate of grid corner
    //! \param[in] isoval iso-value to be interpolated along the line segment
    //! \param[in] dataVals array of floating point values of variable at all grid corners
    //! \return interpolated isovalue
    float interp_i(int i, int j, float isoval, float *dataVals) {
        return (-1. + 2. * (double)(i) / ((double)_sampleSize - 1.)                                                                        //x coordinate at (i,j)
                + 2. / (double)(_sampleSize - 1.)                                                                                          //x grid spacing
                      * (isoval - dataVals[i + _sampleSize * j]) / (dataVals[i + 1 + _sampleSize * (j)] - dataVals[i + _sampleSize * j])); //ratio: 1 is iso at right, 0 if iso at left
    }

    //! Construct the cache of edges associated with a particular isovalue
    //! \param[in] isoval index of isovalue
    //! \param[in] dataVals Array of variable data values for the grid vertices
    //! \param[in] missingVal Missing Value associated with this data
    void buildEdges(int isoval, float *dataVals, float missingVal);

#ifdef DEAD
    //! Attach annotation to the contours (after they have been followed).
    //! \param[in] numcomponents Number of connected components of the contours.
    //! \param[in] iso Index of isovalue being annotated.
    void attachAnnotation(int numcomponents, int iso);
#endif

    //Keys for cache validation.  One for each timestep
    struct cacheKey {
        string varname;
        string hgtVar;
        bool is3D;
        int refLevel;
        int lod;
        vector<double> isovals;
        vector<double> extents;
        bool textEnabled;
        double textDensity;
        vector<double> angles;
    };
    map<int, struct cacheKey *> cacheKeys;

    void updateCacheKey(int timestep);

    void deleteCacheKey(int timestep);

    //for each timestep,there is a pair consisting of the isovalue index and a vector of
    //4 floats (x1,y1,x2,y2) specifying
    //the two endpoints of an contour segment that crosses a cell
    std::map<pair<int, int>, vector<float *>> _lineCache;

    //Each edge is represented as a pair (int, int).  Horizontal edges have both ints positive; vertical have both negative.
    //Vertical edges connecting grid vertices (i,j) and (i,j+1) are represented by the pair (-i, -j-1) (-i <=0; -j-1 < 0)
    //Horizontal edges connecting grid vertices (i,j) and (i+1,j) are represented by the pair (i+1,j) (i+1 > 0, j>=0)
    //

    // _lineCache maps pair<int,int>->vector<float[4]> maps a pair (timestep,isovalueIndex) to a vector of segment endpoints, with one segment for each line in the contour
    //For each segment in the _lineCache, there are 5 temporary mappings:
    // prevEdge maps segments to edges.  Each segment is identified by (timestep, isoval, segmentIndex), corresponding edge is pair(int,int)
    // nextEdge is the same; map(int,int,int)->pair(int,int)
    // plus prevSegment maps prevEdge to segment; i.e. map (timestep, isoval, (int,int))-> segmentIndex
    // similarly nextSegment maps nextEdge to segment; i.e. map (timestep, isoval, (int,int))-> segmentIndex
    // plus forwardEdge maps (timestep,isoval,edge)-> edge
    // and backwardEdge maps (timestep,isoval,edge)-> edge

    std::map<pair<int, int>, int> _edgeSeg; //map an edge to a segment, to find 3d coords where edge is located

    std::map<pair<int, int>, pair<int, int>> _edgeEdge1; //map an edge to one adjacent edge;
    std::map<pair<int, int>, pair<int, int>> _edgeEdge2; //map an edge to other adjacent edge;
    std::map<pair<int, int>, bool> _markerBit;           //indicate whether or not an edge has been visited during current traversal
    vector<int> _componentLength;                        //indicates the number of segments in a component.
    vector<pair<int, int>> _endEdge;                     //indicates and ending edge for each component

    int _numIsovalsCached;
    int _sampleSize;
    std::map<int, int> _objectNums;
};
}; // namespace VAPoR

#endif // CONTOURRENDERER_H
