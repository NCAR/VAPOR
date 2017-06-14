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
#include <vapor/StructuredGrid.h>
#include <vapor/utils.h>
#include <vapor/TwoDDataParams.h>

namespace VAPoR {

class RENDER_API TwoDDataRenderer : public TwoDRenderer
{

public:

 TwoDDataRenderer( 
	const ParamsMgr *pm, string winName,
	string instName, DataStatus *ds
 );

 virtual ~TwoDDataRenderer();

#ifdef	DEAD
 //! CreateInstance:  Static method to create a renderer given the
 //! associated Params instance and visualizer
 //! \param[in] Visualizer* pointer to the visualizer where this will draw
 //! \param[in] RenderParams* pointer to the ArrowParams associated
 //! with this renderer
 //
 static Renderer* CreateInstance(
	const ParamsMgr *pm, string winName,
	string instName, DataStatus *ds
 ) {
    return new TwoDDataRenderer(
		pm, winName, TwoDDataParams::m_classType, ds
	);
 }
#endif

 // Get static string identifier for this render class
 //
 static string GetClassType() {
	return("TwoDData");
 }

protected:
 int _initializeGL();

 int _paintGL();

 int _GetMesh(
	DataMgr *dataMgr,
	GLfloat **verts,
	GLfloat **normals,
	GLsizei &width,
	GLsizei &height
 ); 

 const GLvoid *_GetTexture(
	DataMgr *dataMgr,
	GLsizei &width,
	GLsizei &height,
	GLint &internalFormat,
	GLenum &format,
	GLenum &type,
	size_t &texelSize
 );

	
private:

 GLsizei _texWidth;
 GLsizei _texHeight;
 size_t _texelSize;
 size_t _currentTimestep;
 int _currentRefLevel;
 int _currentLod;
 string _currentVarname;
 vector <double> _currentBoxMinExts;
 vector <double> _currentBoxMaxExts;
 size_t _currentTimestepTex;
 string _currentHgtVar;
 vector <double> _currentBoxMinExtsTex;
 vector <double> _currentBoxMaxExtsTex;
 SmartBuf _sb_verts;
 SmartBuf _sb_normals;
 SmartBuf _sb_texture;
 GLsizei _vertsWidth;
 GLsizei _vertsHeight;

 GLuint _cMapTexID;
 GLfloat *_colormap;
 size_t _colormapsize;

 bool _gridStateDirty() const; 

 void _gridStateClear();

 void _gridStateSet();

 bool _texStateDirty(DataMgr *dataMgr) const; 

 void _texStateSet(DataMgr *dataMgr); 

 void _texStateClear(); 


 int _getMeshDisplaced(
	DataMgr *dataMgr, StructuredGrid *sg, 
	const vector <double> &scaleFac,
	double defaultZ
 ); 

 int _getMeshPlane(
	DataMgr *dataMgr, StructuredGrid *sg,
	const vector <double> &scaleFac,
	double defaultZ
 );

 const GLvoid *_getTexture(DataMgr* dataMgr);


 int _getOrientation( DataMgr *dataMgr, string varname);

};
};

#endif // TWODDATARENDERER_H
