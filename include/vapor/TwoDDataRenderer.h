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
    GLsizei        _texWidth;
    GLsizei        _texHeight;
    size_t         _texelSize;
    size_t         _currentTimestep;
    int            _currentRefLevel;
    int            _currentLod;
    string         _currentVarname;
    vector<double> _currentBoxMinExts;
    vector<double> _currentBoxMaxExts;
    int            _currentRefLevelTex;
    int            _currentLodTex;
    size_t         _currentTimestepTex;
    string         _currentHgtVar;
    vector<double> _currentBoxMinExtsTex;
    vector<double> _currentBoxMaxExtsTex;
    SmartBuf       _sb_verts;
    SmartBuf       _sb_normals;
    SmartBuf       _sb_indices;
    SmartBuf       _sb_texture;
    GLsizei        _vertsWidth;
    GLsizei        _vertsHeight;
    GLsizei        _nindices;

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
};
};    // namespace VAPoR

#endif    // TWODDATARENDERER_H
