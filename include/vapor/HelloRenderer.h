//-- HelloRenderer.h ----------------------------------------------------------
//   
//                   Copyright (C)  2011
//     University Corporation for Atmospheric Research
//                   All Rights Reserved
//
//----------------------------------------------------------------------------
//
//      File:           HelloRenderer.h
//
//      Author:         Alan Norton
//
//
//      Description:  Definition of HelloRenderer class
//
//
//
//----------------------------------------------------------------------------

#ifndef	HELLORENDERER_H
#define	HELLORENDERER_H

#include <vapor/StructuredGrid.h>
#include <vapor/Renderer.h>
#include <vapor/HelloParams.h>

namespace VAPoR {

//! \class HelloRenderer
//! \brief Class that draws a line as specified by HelloParams
//! \author Alan Norton
//! \version 3.0
//! \date June 2015

class RENDER_API HelloRenderer : public Renderer {
	
      
public:

 HelloRenderer(
    const ParamsMgr *pm, string winName,
    string instName, DataStatus *ds
 );

 ~HelloRenderer();

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
    return new HelloRenderer(
        pm, winName, HelloParams::m_classType, ds
    );
 }
#endif


 // Get static string identifier for this Render class
 //
 static string GetClassType() {
	return("Hello");
 }


protected:

 //! \copydoc Renderer::_initializeGL()
 virtual int _initializeGL();

 //! \copydoc Renderer::_paintGL()
 virtual int _paintGL();


private:

};
};

#endif //HELLORENDERER_H
