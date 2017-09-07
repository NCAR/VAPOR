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
#include <vapor/glutil.h> //Must be included first!
#include <vapor/MyBase.h>
#include <vapor/common.h>

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
    //! Default constructor.
    TextObject();

    //! Initialize a text object after creation.
    int Initialize(
        string inFont,
        string inText,
        int inSize,
        float inCoords[3],
        int inType,
        float txtColor[4],
        float bgColor[4]);
    //				QGLWidget *myWindow);
    ~TextObject();

    //! Draw Text Object at default coordinates specified in TextObject::Initialize()
    int drawMe();
    //! Draw Text Object at specified x, y, z coordinate
    int drawMe(float coords[3]);
    //! Draw Text Object at specified x, y, z coordinate, at specified time step
    int drawMe(float coords[3], int timestep);

    float getWidth() { return _width; }

    float getHeight() { return _height; }

    //! Sets the variables \p _width and \p _height.
    //! These define the size of the texture box that text will be drawn on to
    void findBBoxSize(void);

    //! Generates a new framebuffer object and assigns it a unique identifier
    int initFrameBuffer(void);

    //! Saves the current GL state and applies transformations
    //! to draw the appropriate type of text object.
    //void applyViewerMatrix();
    //float * applyViewerMatrix(float coords[3]);
    int applyViewerMatrix(float coords[3] = NULL);

    //! Resets the state of the GL machine to what it was before
    //! our text rendering.
    void removeViewerMatrix();

    void setType(int type) { _type = type; }

  private:
    void initFrameBufferTexture(void);

    GLuint _fbo;
    GLuint _fboTexture;

    int _width;  // Width of our texture
    int _height; // Height of our texture

    FTPixmapFont *_pixmap; // FTGL pixmap object
    string _font;          // font file
    string _text;          // text to display
    int _size;             // font size
    float _txtColor[4];    // text color
    float _bgColor[4];     // background color
    float _coords[3];      // text coordates
    float _3Dcoords[3];    // 3D coordinates used if we draw text within the scene
    int _type;             // in front of scene, following a point in domain, or within
                           // type 0 - Label - text drawn in 2d on top of the scene
                           // type 1 - Billboard - text drawn in 2d, following a point in the scene
                           // type 2 - In-Scene - text drawn within the scene
                           // type 3 - 3D - text drawn within the scene, facing the user
};

class RENDER_API TextContainer {
  public:
    TextContainer(string fontFile,
                  int fontSize,
                  float txtColor[4],
                  float bgColor[4],
                  int type);
    //				QGLWidget *myGivenWindow);
    ~TextContainer();

    int addText(string text,
                float coords[3]);
    void deleteText();
    void drawText();

  private:
    string _font;
    int _size;
    int _type;
    vector<TextObject *> TextObjects;
    //	QGLWidget *_myWindow;
    float _txtColor[4];
    float _bgColor[4];
};

}; // namespace VAPoR

#endif // _TextRenderer_H_
