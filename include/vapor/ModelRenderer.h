//************************************************************************
//                                                                       *
//                          Copyright (C)  2018                          *
//            University Corporation for Atmospheric Research            *
//                          All Rights Reserved                          *
//                                                                       *
//************************************************************************/
//
//  File:   ModelRenderer.cpp
//
//  Author: Stas Jaroszynski
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:   March 2018
//
//  Description:
//          Definition of ModelRenderer
//
#pragma once

#include <GL/glew.h>
#ifdef Darwin
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <vapor/Renderer.h>
#include <vapor/ModelParams.h>

namespace VAPoR {

class DataMgr;
    
//! \class ModelRenderer
//! \brief Class that draws the contours (contours) as specified by IsolineParams
//! \author Stas Jaroszynski
//! \version 1.0
//! \date March 2018
class RENDER_API ModelRenderer : public Renderer
{

public:

	ModelRenderer(const ParamsMgr* pm,
							string winName,
							string dataSetName,
							string instName,
							DataMgr* dataMgr);

	virtual ~ModelRenderer();

	static string GetClassType() {
		return("Model");
	}
	
	//! \copydoc Renderer::_initializeGL()
	virtual int	_initializeGL();
	//! \copydoc Renderer::_paintGL()
    virtual int _paintGL(bool fast);
    virtual void _clearCache() {}
};
    
};
