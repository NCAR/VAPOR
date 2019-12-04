#ifndef VARIABLESWIDGET_H
#define VARIABLESWIDGET_H

#include <QObject>
#include "vapor/MyBase.h"
#include "ui_VariablesWidgetGUI.h"
#include "VaporTable.h"
#include "Flags.h"

QT_USE_NAMESPACE

namespace VAPoR {
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class RenderEventRouter;

//!
//! \class VariablesWidget
//! \ingroup Public_GUI
//! \brief A Widget that can be reused to provide a variable
//! selection tab in any renderer EventRouter class
//! \author Alan Norton
//! \version 3.0
//! \date  June 2015

//!	The VariablesWidget class handles all setting and getting of state in a
//! variables sub-tab of a renderer EventRouter.
//! Implementers of new tabs in vaporgui can insert one of these
//! widgets as a tab in the renderer tab.
//! Implement this as follows:
//!
//! The EventRouter must provide a VariableWidget* as a member of
//! the EventRouter.
//! It is necessary to construct the VariablesWidget in the EventRouter
//! constructor, and add it as a tab to the EventRouter.
//!
//
class VariablesWidget : public QWidget, public Ui_VariablesWidgetGUI {
    Q_OBJECT

public:
    VariablesWidget(QWidget *parent);

    void Reinit(VariableFlags varFlags, DimFlags dimFlags);

    virtual ~VariablesWidget() {}

    //! Respond to user pressing enter after changing text box.
    //! Does nothing since no text boxes.
    virtual void confirmText() {}

    virtual void Update(const VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

    DimFlags GetDimFlags() const;

protected slots:
    //! Respond to selecting the single (primary) variable of field
    void setVarName(const QString &);

    //! Respond to selecting the x component variable of field
    void setXVarName(const QString &);

    //! Respond to selecting the y component variable of field
    void setYVarName(const QString &);

    //! Respond to selecting the z component variable of field
    void setZVarName(const QString &);

    //! Respond to the selection of XY, XZ, or YZ plane orientation
    void set2DOrientation(const QString &);

    //! Respond to selecting the x component variable of seed dist field
    void setXDistVarName(const QString &);

    //! Respond to selecting the y component variable of seed dist field
    void setYDistVarName(const QString &);

    //! Respond to selecting the z component variable of seed dist field
    void setZDistVarName(const QString &);

    //! Respond to selecting the HGT variable
    void setHeightVarName(const QString &);

    //! Respond to choosing the variable dimension
    void setVariableDims(int);

    //! Respond to color-map variable changed
    void setColorMappedVariable(const QString &);

private:
    const VAPoR::DataMgr *_dataMgr;
    VAPoR::ParamsMgr *    _paramsMgr;
    VAPoR::RenderParams * _rParams;

    void pushVarStartingWithLetter(std::vector<string> searchVars, std::vector<string> &returnVars, char letter);

    void setVectorVarName(const QString &name, int component);
    void collapseColorVarSettings();

    void showHideVarCombos(bool on);

    string updateVarCombo(QComboBox *varCombo, const vector<string> &varnames, bool doZero, string currentVar);

    void updateCombos();
    void updateScalarCombo();
    void updateVectorCombo();
    void updateColorCombo();
    void updateHeightCombo();
    void updateDimCombo();

    void setDefaultVariables();
    void setDefaultScalarVar(std::vector<string> vars);
    void setDefaultVectorVar(std::vector<string> vars);
    void setDefaultColorVar(std::vector<string> vars);

    string findVarStartingWithLetter(std::vector<string> searchVars, char letter);

    int _activeDim;

    VariableFlags _variableFlags;
    DimFlags      _dimFlags;
};

#endif    // VARIABLESWIDGET_H
