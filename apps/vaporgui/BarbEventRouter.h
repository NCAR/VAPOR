#ifndef BARBEVENTROUTER_H
#define BARBEVENTROUTER_H

#include <qobject.h>
#include <vapor/MyBase.h>
#include "GL/glew.h"
#include "vapor/BarbRenderer.h"
#include "vapor/BarbParams.h"
#include "RenderEventRouter.h"
#include "TabManager.h"
#include "VariablesWidget.h"
#include "BarbSubtabs.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class GLBarbImageWindow;

//!
//! \class BarbEventRouter
//! \ingroup Public_GUI
//! \brief An EventRouter subclass that handles the Barb tab in the GUI
//! \author Scott Pearse
//! \version 3.0
//! \date  April 2016

//!	The BarbEventRouter class manages the Barb gui.  There are three sub-tabs,
//! for variables, geometry, and appearance.

class BarbEventRouter : public QTabWidget, public RenderEventRouter {

    Q_OBJECT

  public:
    BarbEventRouter(
        QWidget *parent, VizWinMgr *vizMgr, VAPoR::ControlExec *ce);
    ~BarbEventRouter();

    virtual void hookUpTab() {}

    void GetWebHelp(
        vector<pair<string, string>> &help) const;

    //! \copydoc EventRouter::captureMouseUp()
    virtual void captureMouseUp() {}

    //! \copydoc EventRouter::captureMouseDown()
    virtual void captureMouseDown(int button) {}

    //! \copydoc EventRouter::getMappingFrame()
    virtual MappingFrame *getMappingFrame() {
        return _appearance->_TFWidget->mappingFrame;
    }

    //! \copydoc EventRouter::getColorbarFrame()
    virtual ColorbarWidget *getColorbarWidget() {
        return _appearance->_ColorbarWidget;
    }

  private slots:
    //! Respond to georeferencing checkbox being clicked
    void geoCheckboxClicked(bool);

    //! respond when TF editing starts
    void startChangeMapFcn(QString);

    //! respond when TF editing ends
    void endChangeMapFcn();

    //! Respond to image transparency checkbox being clicked
    //void transparencyCheckboxClicked(bool);

    // Get static string identifier for this router class
    //
    static string GetClassType() {
        return (VAPoR::BarbRenderer::GetClassType());
    }

    string GetType() const { return GetClassType(); }

  protected:
    virtual void _updateTab();

  private:
    BarbEventRouter() {}

    //! Override default wheel behavior on the tab.  This has the effect of
    //! ignoring wheel events over the tab.  This is because wheel events
    //! will always affect the combo boxes and other widgets in the tab, and
    //! it would be confusing if wheel events also scrolled the tab itself
    void wheelEvent(QWheelEvent *) {}

    BarbVariablesSubtab *_variables;
    BarbGeometrySubtab *_geometry;
    BarbAppearanceSubtab *_appearance;

    //! Web help text is specified in the class definition
    static const char *_webHelpText[];

    //! Web help URLs are specified in the class definition
    static const char *_webHelpURL[];
};

#endif //BARBEVENTROUTER_H
