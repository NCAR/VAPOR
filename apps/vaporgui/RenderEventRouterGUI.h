#pragma once

#include "RenderEventRouter.h"
#include <vapor/GUIStateParams.h>
#include <QTabWidget>

class ParamsUpdatable;
class UWidget;

//! \class RenderEventRouterGUI
//! \ingroup Public_GUI
//! \brief Tab manager for renderer subtabs
//! \author Stas Jaroszynski

class RenderEventRouterGUI : public QTabWidget, public RenderEventRouter {
    Q_OBJECT

    vector<ParamsUpdatable *> _subtabs;

public:
    static const std::string VariablesTabName;
    static const std::string AppearanceTabName;
    static const std::string GeometryTabName;
    static const std::string AnnotationTabName;

    RenderEventRouterGUI(VAPoR::ControlExec *ce, string paramsType);
    QWidget *AddSubtab(string title, UWidget *subtab);
    QWidget *AddVariablesSubtab(UWidget *subtab) { return AddSubtab(VariablesTabName, subtab); }
    QWidget *AddAppearanceSubtab(UWidget *subtab) { return AddSubtab(AppearanceTabName, subtab); }
    QWidget *AddGeometrySubtab(UWidget *subtab) { return AddSubtab(GeometryTabName, subtab); }
    QWidget *AddAnnotationSubtab(UWidget *subtab) { return AddSubtab(AnnotationTabName, subtab); }


protected:
    void Update() override;

private:
    void            setTab(int i);
    void            tabChanged(int i);
    GUIStateParams *getGUIStateParams() const;
};
