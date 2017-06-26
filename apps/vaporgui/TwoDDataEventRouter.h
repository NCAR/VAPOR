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
 ~TwoDDataEventRouter();

 //! \copydoc EventRouter::hookUpTab()
 void hookUpTab();

 void GetWebHelp(
	vector <pair <string, string> > &help
 ) const;

 //! \copydoc EventRouter::captureMouseUp()
 virtual void captureMouseUp() {}

 //! \copydoc EventRouter::captureMouseDown()
 virtual void captureMouseDown(int button) {}

 //! \copydoc EventRouter::getMappingFrame()
 virtual MappingFrame* getMappingFrame() {return _appearance->_TFWidget->mappingFrame;}

 //! \copydoc EventRouter::getColorbarFrame()
 virtual ColorbarSettings* getColorbarFrame() {return _appearance->_ColorBarFrame;}

 //! \copydoc EventRouter::UpdateMapBounds()
 virtual void UpdateMapBounds();

private slots:

 //! Load a TF from file
 void twodLoadTF();

 //! Load an installed TF
 void twodLoadInstalledTF();

 //! Save the current TF to file
 void twodSaveTF();

 //! respond when TF editing starts
 void startChangeMapFcn(QString);

 //! respond when TF editing ends
 void endChangeMapFcn();

 //! Respond to image transparency checkbox being clicked
 //void transparencyCheckboxClicked(bool);

 // Get static string identifier for this router class
 //
 static string GetClassType() {
	 return(VAPoR::TwoDDataRenderer::GetClassType());
 }
 string GetType() const {return GetClassType(); }


protected:
 void _updateTab();

private:

 TwoDDataEventRouter() {}


 //! Override default wheel behavior on the tab.  This has the effect of 
 //! ignoring wheel events over the tab.  This is because wheel events will always
 //! affect the combo boxes and other widgets in the tab, and it would be confusing
 //! if wheel events also scrolled the tab itself
  void wheelEvent(QWheelEvent*) {}

 //! VariablesWidget is used as Variables tab
 TwoDVariablesSubtab *_variables;
 //	TwoDDataImageGUI *_image;
 TwoDGeometrySubtab* _geometry;
 GLTwoDDataImageWindow* _glTwoDDataImageWindow;
 TwoDAppearanceSubtab * _appearance;

};

#endif //TWODDATAEVENTROUTER_H 


