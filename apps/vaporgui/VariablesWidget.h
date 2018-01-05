#ifndef VARIABLESWIDGET_H
#define VARIABLESWIDGET_H


#include <QObject>
#include "vapor/MyBase.h"
#include "ui_VariablesWidgetGUI.h"
#include "VaporTable.h"


QT_USE_NAMESPACE



namespace VAPoR {
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

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

 //! Bit masks to indicate what type of variables are to be supported by
 //! a particular VariablesWidget instance. These flags correspond
 //! to variable names returned by methods:
 //!
 //! SCALAR : RenderParams::GetVariableName()
 //! VECTOR : RenderParams::GetFieldVariableNames()
 //! HGT : RenderParams::GetHeightVariableName()
 //! COLOR : RenderParams::GetColorMapVariableNames()
 //!
 enum DisplayFlags {
	SCALAR = (1u << 0),
	VECTOR = (1u << 1),
	HGT = (1u << 2),
	COLOR = (1u << 3),
 };

 //! Bit mask to indicate whether 2D, 3D, or 2D and 3D variables are to
 //! be supported
 //
 enum DimFlags {
	TWOD = (1u << 0),
	THREED = (1u << 1),
 };

 enum ColorFlags {
	COLORVAR = (1u << 0),
 };

 VariablesWidget(QWidget* parent);

 void Reinit(
	DisplayFlags dspFlags, 
	DimFlags dimFlags,
	ColorFlags colorFlags);

 virtual ~VariablesWidget(){}

 //! Respond to user pressing enter after changing text box.
 //! Does nothing since no text boxes.
 virtual void confirmText() {}

 virtual void Update(
	const VAPoR::DataMgr *dataMgr,
	VAPoR::ParamsMgr *paramsMgr,
	VAPoR::RenderParams *rParams
 );
 
 string getNDimsTag() {return _nDimsTag;}

protected slots:
 void printTableContents(int row, int col);
 void printTableContents2(int row, int col);

 //! Respond to selecting the single (primary) variable of field 
 void setVarName(const QString&);

 //! Respond to selecting the x component variable of field 
 void setXVarName(const QString&);

 //! Respond to selecting the y component variable of field
 void setYVarName(const QString&);

 //! Respond to selecting the z component variable of field
 void setZVarName(const QString&);

 //! Respond to selecting the x component variable of seed dist field
 void setXDistVarName(const QString&);

 //! Respond to selecting the y component variable of seed dist field
 void setYDistVarName(const QString&);

 //! Respond to selecting the z component variable of seed dist field
 void setZDistVarName(const QString&);

 //! Respond to selecting the HGT variable
 void setHeightVarName(const QString&);

 //! Respond to choosing the variable dimension
 void setVariableDims(int);

 //! Respond to color-map variable changed
 void setColorMappedVariable(const QString&);

private:
 const VAPoR::DataMgr *_dataMgr;
 VAPoR::ParamsMgr *_paramsMgr;
 VAPoR::RenderParams *_rParams;
 
	
 void setVectorVarName(const QString& name, int component);
 void configureDefaultColoring();
 //void configureColorMappingToVariable(string var);
 //void configureConstantColor(string var);
 void configureColorWidgets(string selection);
 void collapseColorVarSettings();

 void showHideVar(bool on);

 string updateVarCombo(
	QComboBox *varCombo, const vector <string> &varnames, bool doZero,
	string currentVar
 );

 void updateVariableCombos(VAPoR::RenderParams* params);

 void updateDims(VAPoR::RenderParams *rParams);

 DisplayFlags _dspFlags;
 DimFlags _dimFlags;
 ColorFlags _colorFlags;

 static string _nDimsTag;
};

#endif //VARIABLESWIDGET_H 
