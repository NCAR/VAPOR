#ifndef HELLOEVENTROUTER_H
#define HELLOEVENTROUTER_H

#include <qobject.h>
#include <vapor/HelloParams.h>
#include <vapor/HelloRenderer.h>
#include <vapor/MyBase.h>

#include "RenderEventRouter.h"
//#include "subeventrouter.h"
#include "helloSubtabs.h"
#include "helloAppearanceSubtab.h"
#include "helloLayoutSubtab.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

//!
//! \class HelloEventRouter
//! \ingroup Public_GUI
//! \brief An RenderEventRouter subclass that handles the Hello tab in the GUI
//! \author Alan Norton
//! \version 3.0
//! \date  June 2015

//!	The HelloEventRouter class manages the Hello gui.  There are three sub-tabs,
//! for variables, geometry, and appearance.

class HelloEventRouter : public QTabWidget, public RenderEventRouter {

    Q_OBJECT

  public:
    HelloEventRouter(
        QWidget *parent, VAPoR::ControlExec *ce);

    virtual ~HelloEventRouter();

    //! \copydoc RenderEventRouter::hookUpTab()
    virtual void hookUpTab();

    virtual void GetWebHelp(
        std::vector<std::pair<string, string>> &help) const;

    // Get static string identifier for this router class
    //
    static string GetClassType() {
        return (VAPoR::HelloRenderer::GetClassType());
    }
    string GetType() const { return GetClassType(); }

  protected slots:

    //! Respond to any text change in hello tabs.
    //! This slot just sets the textChanged flag.
    void setHelloTextChanged(const QString &qs);

    //! Respond to return being pressed over a text box
    //! This just calls confirmText()
    void helloReturnPressed();

    //! Response to selecting a line color.  Launch
    //! a color selector and use it to change the color.
    void selectColor();

  protected:
    //! \copydoc EventRouter::_confirmText()
    virtual void _confirmText();

    //! \copydoc EventRouter::_updateTab()
    virtual void _updateTab();

  private:
    HelloEventRouter() {}

    //Internal classes for sub-widgets:
    //! \class HelloAppearanceGUI
    //! \brief Widget class for Appearance sub-tab
    //class HelloAppearanceGUI : public SubEventRouter, public Ui_helloAppearanceSubtab {
    class HelloAppearanceGUI : public QWidget, public Ui_helloAppearanceSubtab {
      public:
        HelloAppearanceGUI(QWidget *parent, RenderEventRouter *er, const VAPoR::ControlExec *ce) : //SubEventRouter(parent, er, ce), Ui_helloAppearanceSubtab(){
                                                                                                   Ui_helloAppearanceSubtab() {
            //This GUI is a Qt Designer Widget, so call setupUi() to convert the
            //Ui_helloAppearance object into a Widget.
            setupUi(this);
        }

        void Update(
            VAPoR::DataMgr *dataMgr,
            VAPoR::ParamsMgr *paramsMgr,
            VAPoR::RenderParams *rParams);
    };
    //! \class HelloLayoutGUI
    //! \brief Widget class for geometry sub-tab
    class HelloLayoutGUI : public QWidget, public Ui_helloLayoutSubtab {
      public:
        HelloLayoutGUI(QWidget *parent, RenderEventRouter *er, const VAPoR::ControlExec *ce) : //SubEventRouter(parent, er, ce), Ui_helloLayoutSubtab() {
                                                                                               Ui_helloLayoutSubtab() {
            //This GUI is a Qt Designer Widget, so call setupUi() to convert the
            //Ui_helloAppearance object into a Widget.
            setupUi(this);
        }

        void Update(
            VAPoR::DataMgr *dataMgr,
            VAPoR::ParamsMgr *paramsMgr,
            VAPoR::RenderParams *rParams);
    };

    //! Override default wheel behavior on the tab.  This has the effect of
    //! ignoring wheel events over the tab.  This is because wheel events will always
    //! affect the combo boxes and other widgets in the tab, and it would be confusing
    //! if wheel events also scrolled the tab itself
    virtual void wheelEvent(QWheelEvent *) {}

    //! VariablesWidget is used as Variables tab
    HelloVariablesSubtab *_variables;

    //! Appearance tab is a HelloAppearanceGUI QWidget
    HelloAppearanceGUI *_appearance;

    //! Layout tab is a HelloLayoutGui QWidget
    HelloLayoutGUI *_geometry;
};

#endif //HELLOEVENTROUTER_H
