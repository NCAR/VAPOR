//---------------- ----------------------------------------------------------
//
//                   Copyright (C)  2018
//     University Corporation for Atmospheric Research
//                   All Rights Reserved
//
//----------------------------------------------------------------------------

#ifndef WIREFRAMERENDERER_H
#define WIREFRAMERENDERER_H

#include <vapor/glutil.h>    // Must be included first!!!

#ifdef Darwin
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#include <utility>
#include <vapor/DataMgr.h>
#include <vapor/utils.h>
#include <vapor/Renderer.h>

namespace VAPoR {

//! \class WireFrameRenderer
//! \brief
//! \author John Clyne
//! \version 3.0
//! \date June 2018

class RENDER_API WireFrameRenderer : public Renderer {
public:
    //! Constructor, must invoke Renderer constructor
    //! \param[in] Visualizer* pointer to the visualizer where this will draw
    //! \param[in] RenderParams* pointer to the ArrowParams describing
    //! this renderer
    WireFrameRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr);

    static string GetClassType() { return ("WireFrame"); }

    //! Destructor
    //
    virtual ~WireFrameRenderer();

protected:
    //! \copydoc Renderer::_initializeGL()
    virtual int _initializeGL();

    //! \copydoc Renderer::_paintGL()
    virtual int _paintGL(bool fast);

private:
    GLuint       _VAO, _VBO, _EBO;
    unsigned int _nIndices;

    struct VertexData;
    struct {
        string              varName;
        string              heightVarName;
        size_t              ts;
        int                 level;
        int                 lod;
        bool                useSingleColor;
        std::vector<float>  constantColor;
        float               constantOpacity;
        std::vector<float>  tf_lut;
        std::vector<double> tf_minmax;
        std::vector<double> boxMin, boxMax;

    } _cacheParams;

    // Helper class to keep track of which cell edges have been drawn so
    // we can avoid duplicate draws.
    //
    class DrawList {
    public:
        // maxEntries is the maximum number of unique cell nodes.
        // maxLinesPerVertex is the *expected* max valence (degree) of any
        // node (vertex). If a node has more than maxLinesPerVertex edges
        // only the first maxLinesPerVertex edges will be recorded in
        // DrawList. Hence, queries to edges with DrawList::InList will
        // return false once maxLinesPerVertex has been exceeded
        //
        DrawList(GLuint maxEntries, size_t maxLinesPerVertex)
        : _drawList(maxEntries * maxLinesPerVertex, std::numeric_limits<GLuint>::max()), _maxEntries(maxEntries), _maxLinesPerVertex(maxLinesPerVertex)
        {
        }

        bool InList(GLuint idx0, GLuint idx1)
        {
            VAssert(idx0 < _maxEntries);
            VAssert(idx1 < _maxEntries);

            if (idx1 < idx0) { std::swap(idx1, idx0); }

            for (int i = 0; i < _maxLinesPerVertex; i++) {
                if (_drawList[idx0 * _maxLinesPerVertex + i] == idx1) { return (true); }
                if (_drawList[idx0 * _maxLinesPerVertex + i] == std::numeric_limits<GLuint>::max()) {
                    _drawList[idx0 * _maxLinesPerVertex + i] = idx1;
                    return (false);
                }
            }
            return (false);
        }

    private:
        vector<GLuint> _drawList;
        const size_t   _maxEntries;
        const size_t   _maxLinesPerVertex;
    };

    void _buildCacheVertices(const Grid *grid, const Grid *heightGrid, vector<GLuint> &nodeMap) const;

    size_t _buildCacheConnectivity(const Grid *grid, const vector<GLuint> &nodeMap) const;

    int  _buildCache();
    bool _isCacheDirty() const;
    void _saveCacheParams();
    void _drawCell(const GLuint *cellNodeIndices, int n, bool layered, const std::vector<GLuint> &nodeMap, GLuint invalidIndex, std::vector<unsigned int> &indices, DrawList &drawList) const;

    void _clearCache() { _cacheParams.varName.clear(); }
};

};    // namespace VAPoR

#endif
