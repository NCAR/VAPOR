//---------------------------------------------------------------------------
//   
//                   Copyright (C)  2011
//     University Corporation for Atmospheric Research
//                   All Rights Reserved
//
//----------------------------------------------------------------------------

#include <vapor/glutil.h>	// Must be included first!!!
#include <cstdlib>
#include <cstdio>


#include <vapor/ViewpointParams.h>
#include <vapor/MyBase.h>
#include <vapor/TwoDRenderer.h>
#include "vapor/GLManager.h"

using namespace VAPoR;
using namespace Wasp;

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TwoDRenderer::TwoDRenderer(
	const ParamsMgr *pm, string winName, string dataSetName, string paramsType,
	string classType, string instName, DataMgr *dataMgr
) : Renderer(pm, winName, dataSetName, paramsType, classType, instName, dataMgr) {
	_textureID = 0;
	_texture = NULL;
	_texCoords = NULL;
	_texWidth = 0;
	_texHeight = 0;
	_texInternalFormat = 0;
	_texFormat = GL_RGBA;
	_texType = GL_UNSIGNED_BYTE;
	_texelSize = 0;
	_gridAligned = false;
	_structuredMesh = false;
	_verts = NULL;
	_normals = NULL;
	_meshWidth = 0;
	_meshHeight = 0;
    _VAO = (int)NULL;
    _VBO = (int)NULL;
    _dataVBO = (int)NULL;
    _EBO = (int)NULL;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TwoDRenderer::~TwoDRenderer()
{
	if (_textureID) glDeleteTextures(1, &_textureID);
    if (_VAO) glDeleteVertexArrays(1, &_VAO);
    if (_VBO) glDeleteBuffers(1, &_VBO);
    if (_dataVBO) glDeleteBuffers(1, &_dataVBO);
    if (_EBO) glDeleteBuffers(1, &_EBO);
}

int TwoDRenderer::_initializeGL(){
	glGenTextures(1, &_textureID);
    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    glGenBuffers(1, &_dataVBO);
    glGenBuffers(1, &_EBO);
    
    glBindVertexArray(_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), NULL);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, _dataVBO);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), NULL);
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	return(0);
}

int TwoDRenderer::_paintGL(bool) {

	// Get the 2D texture
	//
	_texture = GetTexture(
		_dataMgr, _texWidth, _texHeight, _texInternalFormat,
		_texFormat, _texType, _texelSize, _gridAligned
	);
	if (! _texture) {
		return(-1);
	}
	VAssert(_texWidth >= 1);
	VAssert(_texHeight >= 1);

	// Get the proxy geometry used to render the 2D surface (vertices and
	// normals)
	//
	int rc = GetMesh(
		_dataMgr, &_verts, &_normals, _nverts, _meshWidth, _meshHeight,
		&_indices, _nindices, _structuredMesh
	);
	if (rc < 0) {
		return(-1);
	}

	if (! _gridAligned) {
		VAssert(_structuredMesh);
		VAssert(_meshWidth >= 2);
		VAssert(_meshHeight >= 2);
	
        _texCoords = (GLfloat *) _sb_texCoords.Alloc(_meshWidth * _meshHeight * 2 * sizeof(*_texCoords));
		_computeTexCoords(_texCoords, _meshWidth, _meshHeight);

		_renderMeshUnAligned();
	}
	else {
		VAssert(_meshWidth == _texWidth);
		VAssert(_meshHeight == _texHeight);

		_renderMeshAligned();
	}
    
    GL_ERR_BREAK();

	return(0);
}

void TwoDRenderer::_openGLInit() {
	if (! _gridAligned) {
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _textureID);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, _texInternalFormat, _texWidth, _texHeight, 0, _texFormat, _texType, _texture);
	}

    _glManager->matrixManager->MatrixModeModelView();

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthMask(GL_TRUE);
}

void TwoDRenderer::_openGLRestore()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
}

void TwoDRenderer::_renderMeshUnAligned() {
    // This appears to be pretty much the Image Renderer code
    
    RenderParams *myParams = (RenderParams *) GetActiveParams();
    float opacity = myParams->GetConstantOpacity();
    
    ShaderProgram *shader = _glManager->shaderManager->GetShader("Image");
    if (shader == nullptr)
        return;
    shader->Bind();
    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    shader->SetUniform("constantOpacity", opacity);
    
    EnableClipToBox(shader);
	_openGLInit();
    
    int W = _meshWidth;
    int H = _meshHeight;

    glBindVertexArray(_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * W * sizeof(GLuint), _indices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, H * W * 3*sizeof(float), _verts, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, _dataVBO);
    glBufferData(GL_ARRAY_BUFFER, H * W * 2*sizeof(float), _texCoords, GL_DYNAMIC_DRAW);
    
	for (int j=0; j < H-1; j++)
        glDrawElementsBaseVertex(GL_TRIANGLE_STRIP, 2 * W, GL_UNSIGNED_INT, 0, j*W);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	_openGLRestore();
    DisableClippingPlanes();
}

void TwoDRenderer::_renderMeshAligned() {
    RenderParams *myParams = (RenderParams *) GetActiveParams();
    float opacity = myParams->GetConstantOpacity();
    
    ShaderProgram *shader = _glManager->shaderManager->GetShader("2DData");
    if (shader == nullptr)
        return;
    shader->Bind();
    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    shader->SetUniform("constantOpacity", opacity);

    EnableClipToBox(shader);
	_openGLInit();

	// Ugh. For aligned data the type must be GLfloat.
	//
	VAssert (_texType == GL_FLOAT);
	VAssert (_texelSize == 8);
	const GLfloat *data = (GLfloat *) _texture;

	if (_structuredMesh) {
		// Draw triangle strips one row at a time
		//
        glBindVertexArray(_VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*_meshWidth*sizeof(GLuint), _indices, GL_DYNAMIC_DRAW);
		for (int j=0; j<_meshHeight-1; j++) {
            glBindBuffer(GL_ARRAY_BUFFER, _VBO);
            glBufferData(GL_ARRAY_BUFFER, _meshWidth*6*sizeof(float), &_verts[j*_meshWidth*3], GL_STREAM_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, _dataVBO);
            glBufferData(GL_ARRAY_BUFFER, _meshWidth*4*sizeof(float), &data[j*_meshWidth*2], GL_STREAM_DRAW);
            glDrawElements(GL_TRIANGLE_STRIP, 2*_meshWidth, GL_UNSIGNED_INT, 0);
		}
	}
	else {
		VAssert(_meshWidth >= 3);
		VAssert(_meshHeight == 1);
		VAssert((_nindices % 3) == 0);
        
        glBindVertexArray(_VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _nindices*sizeof(GLsizei), _indices, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, _nverts*3*sizeof(float), _verts, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, _dataVBO);
        glBufferData(GL_ARRAY_BUFFER, _nverts*2*sizeof(float), data, GL_STREAM_DRAW);
        // glNormalPointer(GL_FLOAT, 0, _normals);
        glDrawElements(GL_TRIANGLES, _nindices, GL_UNSIGNED_INT, 0);
	}
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	_openGLRestore();
    DisableClippingPlanes();
}


void TwoDRenderer::ComputeNormals(
	const GLfloat *verts,
	GLsizei w, GLsizei h,
	GLfloat *normals
) {
	// Go over the grid of vertices, calculating normals
	// by looking at adjacent x,y,z coords.
	//
	for (int j = 0; j < h; j++){
	for (int i = 0; i< w; i++){
		const GLfloat* point = verts+3*(i+w*j);
		GLfloat* norm = normals+3*(i+w*j);
		//do differences of right point vs left point,
		//except at edges of grid just do differences
		//between current point and adjacent point:
		float dx=0.f, dy=0.f, dzx=0.f, dzy=0.f;
		if (i>0 && i <w-1){
			dx = *(point+3) - *(point-3);
			dzx = *(point+5) - *(point-1);
		} else if (i == 0) {
			dx = *(point+3) - *(point);
			dzx = *(point+5) - *(point+2);
		} else if (i == w-1) {
			dx = *(point) - *(point-3);
			dzx = *(point+2) - *(point-1);
		}
		if (j>0 && j <h-1){
			dy = *(point+1+3*w) - *(point+1 - 3*w);
			dzy = *(point+2+3*w) - *(point+2 - 3*w);
		} else if (j == 0) {
			dy = *(point+1+3*w) - *(point+1);
			dzy = *(point+2+3*w) - *(point+2);
		} else if (j == h-1) {
			dy = *(point+1) - *(point+1 - 3*w);
			dzy = *(point+2) - *(point+2 - 3*w);
		}
		norm[0] = dy*dzx;
		norm[1] = dx*dzy;
		norm[2] = 1.0;

		//vnormal(norm);
	}
	}
}

void TwoDRenderer::_computeTexCoords(
	GLfloat *tcoords, 
	size_t w, 
	size_t h
) const {
	VAssert(_meshWidth >= 2);
	VAssert(_meshHeight >= 2);

	double deltax = 1.0 / (_meshWidth - 1);
	double deltay = 1.0 / (_meshHeight - 1);

	for (int j=0; j<h; j++) {
	for (int i=0; i<w; i++) {
		tcoords[(2*j*w) + (2*i+0)] = i * deltax;
		tcoords[(2*j*w) + (2*i+1)] = j * deltay;
	}
	}
}

