#ifndef TWODDATAEVENTROUTER_H
#define TWODDATAEVENTROUTER_H


#include <qobject.h>
#include <vapor/MyBase.h>
#include "GL/glew.h"
#include "vapor/TwoDDataRenderer.h"
#include "vapor/TwoDDataParams.h"
#include "RenderEventRouter.h"
#include "VariablesWidget.h"
#include "TwoDSubtabs.h"

QT_USE_NAMESPACE

namespace VAPoR {
	class ControlExec;
}

class GLTwoDDataImageWindow;

//!
//! \class TwoDDataEventRouter
//! \ingroup Public_GUI
//! \brief An EventRouter subclass that handles the TwoD tab in the GUI
//! \author Scott Pearse 
//! \version 3.0
//! \date  April 2016

//!	The TwoDDataEventRouter class manages the TwoD gui.  There are three sub-tabs,
//! for variables, geometry, and appearance. 

class TwoDDataEventRouter : public QTabWidget,  public RenderEventRouter {

Q_OBJECT

public: 

 TwoDDataEventRouter(
	QWidget *parent, VAPoR::ControlExec *ce
 );

 void GetWebHelp(
	vector <pair <string, string> > &help
 ) const;

 //
 static string GetClassType() {
	 return(VAPoR::TwoDDataRenderer::GetClassType());
 }
 string GetType() const {return GetClassType(); }

 virtual bool Supports2DVariables() const { return true; }
 virtual bool Supports3DVariables() const { return false; }    

protected:
 void _updateTab();
 virtual string _getDescription() const;
                      
 virtual string _getSmallIconImagePath() const {
	return("TwoDData_small.png");
 }   
 virtual string _getIconImagePath() const {
	return("TwoDData.png");
 }
    
private:

 TwoDDataEventRouter() {} 


 //! Override default wheel behavior on the tab.  This has the effect of 
 //! ignoring wheel events over the tab.  This is because wheel events will always
 //! affect the combo boxes and other widgets in the tab, and it would be confusing
 //! if wheel events also scrolled the tab itself
  void wheelEvent(QWheelEvent*) {}

 TwoDVariablesSubtab* _variables;
 TwoDGeometrySubtab* _geometry;
 GLTwoDDataImageWindow* _glTwoDDataImageWindow;
 TwoDAppearanceSubtab* _appearance;
 TwoDAnnotationSubtab* _annotation;

#ifdef VAPOR3_0_0_ALPHA
 	TwoDDataImageGUI *_image;
#endif

};

#endif //TWODDATAEVENTROUTER_H 
