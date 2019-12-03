#ifndef SLICEEVENTROUTER_H
#define SLICEEVENTROUTER_H

#include <qobject.h>
#include <vapor/MyBase.h>
#include "GL/glew.h"
#include "vapor/SliceRenderer.h"
#include "vapor/SliceParams.h"
#include "RenderEventRouter.h"
#include "VariablesWidget.h"
#include "SliceSubtabs.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class GLSliceImageWindow;

//!
//! \class SliceEventRouter
//! \ingroup Public_GUI
//! \brief An EventRouter subclass that handles the Slice tab in the GUI
//! \author Scott Pearse
//! \version 3.0
//! \date  April 2016

//!	The SliceEventRouter class manages the Slice gui.  There are three sub-tabs,
//! for variables, geometry, and appearance.

class SliceEventRouter : public QTabWidget, public RenderEventRouter {
    Q_OBJECT

public:
    SliceEventRouter(QWidget *parent, VAPoR::ControlExec *ce);

    void GetWebHelp(vector<pair<string, string>> &help) const;

    //
    static string GetClassType() { return (VAPoR::SliceRenderer::GetClassType()); }
    string        GetType() const { return GetClassType(); }

protected:
    void           _updateTab();
    virtual string _getDescription() const;

    virtual string _getSmallIconImagePath() const { return ("Slice_small.png"); }
    virtual string _getIconImagePath() const { return ("Slice.png"); }

private:
    SliceEventRouter() {}

    //! Override default wheel behavior on the tab.  This has the effect of
    //! ignoring wheel events over the tab.  This is because wheel events will always
    //! affect the combo boxes and other widgets in the tab, and it would be confusing
    //! if wheel events also scrolled the tab itself
    void wheelEvent(QWheelEvent *) {}

    //! VariablesWidget is used as Variables tab
    SliceVariablesSubtab * _variables;
    SliceGeometrySubtab *  _geometry;
    GLSliceImageWindow *   _glSliceImageWindow;
    SliceAppearanceSubtab *_appearance;
    SliceAnnotationSubtab *_annotation;

#ifdef VAPOR3_0_0_ALPHA
    SliceImageGUI *_image;
#endif
};

#endif    // SLICEEVENTROUTER_H
