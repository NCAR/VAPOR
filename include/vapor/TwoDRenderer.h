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

 // Protected pure virual methods
 //

 // Return a 2D structured or unstructured mesh
 //
 // verts : contains a packed representation of the x,y,z coordinates 
 // of each grid point. Thus size is height * width * sizeof(float) * 3
 //
 // normals : contains surface normal at each vertex. Need not be unit length
 // Same packing as verts
 //
 // nverts : number of vertices and number of normals in verts, and 
 // normals, respectively. A single vertex or normal consists of three
 // components. Thus if nverts == 1 then verts and normals each contain
 // one three-component element.
 //
 // width : For structured grids contains number of grid points along
 // fastest varying dimension. For unstructured grids contains *total*
 // number of grid points
 //
 // height : For structured grids contains number of grid points along
 // second fastest varying dimension. For unstructured grids should be set to 
 // one.
 //
 // indices : indexes into verts and normals to generate either triangles
 // (unstructured mesh) or triangle strips (structured mesh). 
 // Compatible with index argument to GLDrawElements
 //
 // nindices : num elements in indices
 //
 // structuredMesh : bool, true if structured mesh, false if unstructured
 //
 virtual int GetMesh(  DataMgr *dataMgr,
                        GLfloat **verts,
                        GLfloat **normals,
                        GLsizei &nverts,
                        GLsizei &width,
                        GLsizei &height,
                        GLuint **indices,
                        GLsizei &nindices,
                        bool &structuredMesh) = 0;

 // Return data values for mesh returned with GetMesh(). The returned
 // array may or may not be coincident with the mesh nodes. In the latter
 // case the array returned is a uniformally 2D sampling of the data
 // values on the mesh. 
 //
 // width : For grid aligned data (gridAligned == true) contains number of 
 // data values along
 // fastest varying dimension. For non-aligned data (gridAligned == false) 
 // contains *total* number of elements
 //
 // height : For grid aligned data (gridAligned == true) contains number of 
 // data values along
 // second fastest varying dimension. For non-aligned data 
 // (gridAligned == false) should be set to one.
 //
 // type : Type of data returned by GetTexture(). If gridAligned
 // is true, type must be GL_FLOAT
 //
 // texelSize: Size, in bytes, of a single element returned by GetTexture.
 //
 // gridAligned : bool. If true data are coincident with mesh returned by
 // GetMesh()
 //
 virtual const GLvoid *GetTexture( DataMgr *dataMgr,
                                    GLsizei &width,
                                    GLsizei &height,
                                    GLint &internalFormat,
                                    GLenum &format,
                                    GLenum &type,
                                    size_t &texelSize,
                                    bool &gridAligned) = 0;

 virtual void _clearCache() = 0;

 //! \copydoc Renderer::_initializeGL()
 virtual int _initializeGL();

 //! \copydoc Renderer::_paintGL()
 virtual int _paintGL(bool fast);

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
 void ComputeNormals(
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
 GLsizei _nverts;
 SmartBuf _sb_texCoords;
    
 GLuint _VAO, _VBO, _dataVBO, _EBO;
 
 
 void _openGLInit();
 void _openGLRestore();
 void _renderMesh();
 void _renderMeshUnAligned();
 void _renderMeshAligned();
 void _computeTexCoords( GLfloat *tcoords, size_t w, size_t h) const;

};
};

#endif
