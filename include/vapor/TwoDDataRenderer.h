//************************************************************************
//																		*
//		     Copyright (C)  2008										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//																		*
//************************************************************************/
//
//	File:		TwoDDataRender.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		March 2009
//
//	Description:	Definition of the TwoDDataRenderer class
//
#ifndef TWODDATARENDERER_H
#define TWODDATARENDERER_H

#include <GL/glew.h>

#ifdef Darwin
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <vapor/TwoDRenderer.h>
#include <vapor/DataMgr.h>
#include <vapor/GeoImage.h>
#include <vapor/Grid.h>
#include <vapor/utils.h>
#include <vapor/TwoDDataParams.h>

namespace VAPoR {

class RENDER_API TwoDDataRenderer : public TwoDRenderer {
public:
    TwoDDataRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr);

    virtual ~TwoDDataRenderer();

    // Get static string identifier for this render class
    //
    static string GetClassType() { return ("TwoDData"); }

protected:
    int _initializeGL();

    int _paintGL();

    int GetMesh(DataMgr *dataMgr, GLfloat **verts, GLfloat **normals, GLsizei &width, GLsizei &height, GLuint **indices, GLsizei &nindices, bool &structuredMesh);

    const GLvoid *GetTexture(DataMgr *dataMgr, GLsizei &width, GLsizei &height, GLint &internalFormat, GLenum &format, GLenum &type, size_t &texelSize, bool &gridAligned);

    virtual GLuint GetAttribIndex() const { return (_vertexDataAttr); }

private:
    class _grid_state_c {
    public:
        _grid_state_c() = default;
        _grid_state_c(int refLevel, int lod, string hgtVar, string meshName, size_t ts, vector<double> minExts, vector<double> maxExts)
        : _refLevel(refLevel), _lod(lod), _hgtVar(hgtVar), _meshName(meshName), _ts(ts), _minExts(minExts), _maxExts(maxExts)
        {
        }

        void clear()
        {
            _refLevel = _lod = -1;
            _hgtVar = _meshName = "";
            _ts = 0;
            _minExts.clear();
            _maxExts.clear();
        }

        bool operator==(const _grid_state_c &rhs) const
        {
            return (_refLevel == rhs._refLevel && _lod == rhs._lod && _hgtVar == rhs._hgtVar && _meshName == rhs._meshName && _ts == rhs._ts && _minExts == rhs._minExts && _maxExts == rhs._maxExts);
        }
        bool operator!=(const _grid_state_c &rhs) const { return (!(*this == rhs)); }

    private:
        int            _refLevel;
        int            _lod;
        string         _hgtVar;
        string         _meshName;
        size_t         _ts;
        vector<double> _minExts;
        vector<double> _maxExts;
    };

    class _tex_state_c {
    public:
        _tex_state_c() = default;
        _tex_state_c(int refLevel, int lod, string varname, size_t ts, vector<double> minExts, vector<double> maxExts)
        : _refLevel(refLevel), _lod(lod), _varname(varname), _ts(ts), _minExts(minExts), _maxExts(maxExts)
        {
        }

        void clear()
        {
            _refLevel = _lod = -1;
            _varname = "";
            _ts = 0;
            _minExts.clear();
            _maxExts.clear();
        }

        bool operator==(const _tex_state_c &rhs) const
        {
            return (_refLevel == rhs._refLevel && _lod == rhs._lod && _varname == rhs._varname && _ts == rhs._ts && _minExts == rhs._minExts && _maxExts == rhs._maxExts);
        }
        bool operator!=(const _tex_state_c &rhs) const { return (!(*this == rhs)); }

    private:
        int            _refLevel;
        int            _lod;
        string         _varname;
        size_t         _ts;
        vector<double> _minExts;
        vector<double> _maxExts;
    };

    _grid_state_c _grid_state;
    _tex_state_c  _tex_state;

    GLsizei  _texWidth;
    GLsizei  _texHeight;
    size_t   _texelSize;
    SmartBuf _sb_verts;
    SmartBuf _sb_normals;
    SmartBuf _sb_indices;
    SmartBuf _sb_texture;
    GLsizei  _vertsWidth;
    GLsizei  _vertsHeight;
    GLsizei  _nindices;

    GLuint   _cMapTexID;
    GLfloat *_colormap;
    size_t   _colormapsize;
    GLuint   _vertexDataAttr;

    bool _gridStateDirty() const;

    void _gridStateClear();

    void _gridStateSet();

    bool _texStateDirty(DataMgr *dataMgr) const;

    void _texStateSet(DataMgr *dataMgr);

    void _texStateClear();

    int _getMeshStructured(DataMgr *dataMgr, const StructuredGrid *g, double defaultZ);

    int _getMeshUnStructured(DataMgr *dataMgr, const Grid *g, double defaultZ);

    int _getMeshUnStructuredHelper(DataMgr *dataMgr, const Grid *g, double defaultZ);

    int _getMeshStructuredDisplaced(DataMgr *dataMgr, const StructuredGrid *g, double defaultZ);

    int _getMeshStructuredPlane(DataMgr *dataMgr, const StructuredGrid *g, double defaultZ);

    const GLvoid *_getTexture(DataMgr *dataMgr);

    int _getOrientation(DataMgr *dataMgr, string varname);

    double _getDefaultZ(DataMgr *dataMgr, size_t ts) const;
};
};    // namespace VAPoR

#endif    // TWODDATARENDERER_H
