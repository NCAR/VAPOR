//---------------- ----------------------------------------------------------
//   
//                   Copyright (C)  2016
//     University Corporation for Atmospheric Research
//                   All Rights Reserved
//
//----------------------------------------------------------------------------

#ifndef	TWODRENDERER_H
#define	TWODRENDERER_H

#include <vapor/glutil.h> // Must be included first!!!

#ifdef Darwin
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <vapor/DataMgr.h>
#include <vapor/utils.h>
#include <vapor/Renderer.h>

namespace VAPoR {

//! \class TwoDRenderer
//! \brief 
//! \author John Clyne
//! \version 3.0
//! \date March 2016

class RENDER_API TwoDRenderer : public Renderer 
{
public:

 //! Constructor, must invoke Renderer constructor
 //! \param[in] Visualizer* pointer to the visualizer where this will draw
 //! \param[in] RenderParams* pointer to the ArrowParams describing 
 //! this renderer
 TwoDRenderer(  const ParamsMgr*    pm, 
                      string        winName,     
                      string        dataSetName, 
                      string        paramsType,
	                    string        classType, 
                      string        instName,                 
                      DataMgr*      dataMgr );

 //! Destructor
 //
 virtual ~TwoDRenderer();


protected:

 virtual int _getMesh(  DataMgr *dataMgr,
                        GLfloat **verts,
                        GLfloat **normals,
                        GLsizei &width,
                        GLsizei &height,
                        GLuint **indices,
                        GLsizei &nindices,
                        bool &structuredMesh) = 0;

 virtual const GLvoid *_getTexture( DataMgr *dataMgr,
                                    GLsizei &width,
                                    GLsizei &height,
                                    GLint &internalFormat,
                                    GLenum &format,
                                    GLenum &type,
                                    size_t &texelSize,
                                    bool &gridAligned) = 0;

 virtual GLuint _getAttribIndex() const = 0;


 //! \copydoc Renderer::_initializeGL()
 virtual int _initializeGL();

 //! \copydoc Renderer::_paintGL()
 virtual int _paintGL();

 //! Compute 2D surface normals at each vertex.
 //!
 //! This protected method can be used by derived classes to calculate
 //! unitized normals from a set of vertices by calculating the gradient
 //! at each vertex.  The normal vectors are determined by looking 
 //! at z-coords of adjacent vertices in both 
 //! x and y.  Suppose that dzx is the change in z associated with 
 //! an x-change of dz, and that dzy is the change in z associated with 
 //! a y-change of dy.  Let the normal
 //! vector be (a,b,c).  Then a/c is equal to dzx/dx, and b/c is 
 //! equal to dzy/dy.  So (a,b,c) is proportional to (dzx*dy, dzy*dx, 1) 
 //!
 //! \param[in] verts A pointer to a 2D array of surface vertices, dimensioned
 //! \p w by \p h by 3. Vertices are store as inteleaved 
 //! triplets: vx0, vy0, vz0, vx1, vy1, vz1, ... vxn, vyn, vzn, where 
 //! \b n = \p w * \p h - 1
 //!
 //! \param[in] w Width (length of fastest varying dimension) 
 //! of \p verts in grid points 
 //! \param[in] h Height (length of second fastest varying dimension) 
 //! of \p verts in grid points 
 //!
 //! \param[out] normals A pointer to a 2D array of the same size as \p verts
 //! that will contain the computed, unitized surface normals, stored
 //! in interleaved form.
 //
 void _ComputeNormals(
	const GLfloat *verts,
	GLsizei w, GLsizei h,
	GLfloat *normals
 );

private:

 GLuint _textureID;
 const GLvoid *_texture;
 GLfloat *_texCoords;
 GLsizei _texWidth;
 GLsizei _texHeight;
 GLint _texInternalFormat;
 GLenum _texFormat;
 GLenum _texType;
 size_t _texelSize;
 bool _gridAligned;
 bool _structuredMesh;
 GLfloat *_verts;
 GLfloat *_normals;
 GLuint *_indices;
 GLsizei _meshWidth;
 GLsizei _meshHeight;
 GLsizei _nindices;
 SmartBuf _sb_texCoords;
 
 
 void _openGLInit();
 void _openGLRestore();
 void _renderMesh();
 void _renderMeshUnAligned();
 void _renderMeshAligned();
 void _computeTexCoords( GLfloat *tcoords, size_t w, size_t h) const;

};
};

#endif
