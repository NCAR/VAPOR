//************************************************************************
//																	   *
//		   Copyright (C)  2004										 *
//	 University Corporation for Atmospheric Research				   *
//		   All Rights Reserved										 *
//																	   *
//************************************************************************/
//
//  File:		textRenderer.h
//
//  Author:	  Scott Pearse
//			   National Center for Atmospheric Research
//			   PO 3000, Boulder, Colorado
//
//  Date:		June 2014
//
//  Description: Implementation of text containers that render to
//			   QGLWidgets
//

#ifndef _TextRenderer_H_
#define _TextRenderer_H_
#include <vapor/glutil.h>    //Must be included first!
#include <vapor/MyBase.h>
#include <vapor/common.h>
#include <vapor/ParamsMgr.h>
#include <vapor/ViewpointParams.h>

class FTPixmapFont;

//
//! \class TextObject
//! \A container class for an FTGL font and its texture for display
//! \author Scott Pearse
//! \version $Revision$
//! \date	$Date$
//!
//! This class provides an API for displaying a single texture that
//! contains text generated in FTGL.

namespace VAPoR {
class RENDER_API TextObject {
public:
    enum OrientationFlag {
        BOTTOMLEFT = (1u << 0),
        CENTERLEFT = (1u << 1),
        TOPLEFT = (1u << 2),
        CENTERTOP = (1u << 3),
        TOPRIGHT = (1u << 4),
        CENTERRIGHT = (1u << 5),
        BOTTOMRIGHT = (1u << 6),
        CENTERBOTTOM = (1u << 7),
    };

    enum TypeFlag {
        LABEL = (1u << 0),
        BILLBOARD = (1u << 1),
        INSCENE = (1u << 2),
        THREED = (1u << 3),
    };

    TextObject();

    int Initialize(string inFont, string inText, int inSize, double inCoords[3], TypeFlag inType, double txtColor[4], double bgColor[4], ViewpointParams *vpParams = NULL);
    ~TextObject();

    //! Draw Text Object at specified x, y, z coordinate
    int drawMe(double coords[3]);
    //! Draw Text Object at specified x, y, z coordinate, at specified time step
    int drawMe(double coords[3], int timestep);

    double getWidth() { return _width; }

    double getHeight() { return _height; }

    //! Sets the variables \p _width and \p _height.
    //! These define the size of the texture box that text will be drawn on to
    void findBBoxSize(void);

    //! Generates a new framebuffer object and assigns it a unique identifier
    int initFrameBuffer(void);

    //! Saves the current GL state and applies transformations
    //! to draw the appropriate type of text object.
    // void applyViewerMatrix();
    // float * applyViewerMatrix(float coords[3]);
    int applyViewerMatrix(double coords[3] = NULL);

    //! Resets the state of the GL machine to what it was before
    //! our text rendering.
    void removeViewerMatrix();

    void setType(int type) { _type = type; }

private:
    void initFrameBufferTexture(void);
    bool projectPointToWin(double coords[3], double newCoords[3]);
    bool projectPointToWin(float coords[3], double newCoords[3]);
    void drawLabel();
    void drawBillboard();

    GLuint _fbo;
    GLuint _fboTexture;

    int _width;     // Width of our texture
    int _height;    // Height of our texture

    OrientationFlag _orientationFlag;
    TypeFlag        _typeFlag;

    ViewpointParams *_vpParams;
    FTPixmapFont *   _pixmap;         // FTGL pixmap object
    string           _font;           // font file
    string           _text;           // text to display
    int              _size;           // font size
    double           _txtColor[4];    // text color
    double           _bgColor[4];     // background color
    double           _coords[3];      // text coordates
    double           _3Dcoords[3];    // 3D coordinates used if we draw text within the scene

    int _type;
    // type 0 - Label - text drawn in 2d on top of the scene.
    //		Takes pixel coordinates.
    // type 1 - Billboard - text drawn in 2d, following a point in the scene
    // type 2 - In-Scene - text drawn within the scene
    // type 3 - 3D - text drawn within the scene, facing the user
    // types 1, 2, and 3 take world coordinates
};
};    // namespace VAPoR

#endif
