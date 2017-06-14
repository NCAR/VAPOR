#ifndef VARIABLESWIDGET_H
#define VARIABLESWIDGET_H


#include <QObject>
#include "vapor/MyBase.h"
#include "VariablesWidgetGUI.h"

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

 VariablesWidget(QWidget* parent);

 void Reinit(
	DisplayFlags dspFlags, DimFlags dimFlags);

 virtual ~VariablesWidget(){}

 //! Respond to user pressing enter after changing text box.
 //! Does nothing since no text boxes.
 virtual void confirmText() {}

 virtual void Update(
	const VAPoR::DataMgr *dataMgr,
	VAPoR::ParamsMgr *paramsMgr,
	VAPoR::RenderParams *rParams
 );

protected slots:
 //! Connected to the image file text editor
 void setNumRefinements(int num);

 //! Connected to the compression ratio selector, setting the lod index.
 void setCompRatio(int num);

 //! Connected to the fidelity button selector, setting the fidelity index.
 void setFidelity(int buttonID);

 //! Connected to the fidelity setDefault button, setting current 
 //! fidelity as default
 void SetFidelityDefault();

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

private:
 const VAPoR::DataMgr *_dataMgr;
 VAPoR::ParamsMgr *_paramsMgr;
 VAPoR::RenderParams *_rParams;
 
	
 void setVectorVarName(const QString& name, int component);

 // Get the compression rates as a fraction for both the LOD and
 // Refinment parameters. Also format these factors into a displayable
 // string
 //
 void getCmpFactors(
	string varname, vector <float> &lodCF, vector <string> &lodStr,
	vector <float> &multiresCF, vector <string> &multiresStr
 ) const;

 void uncheckFidelity();

 void setupFidelity(
	VAPoR::RenderParams* dParams
#ifdef	DEAD
	, bool useDefault
#endif
 );

 //! Set the fidelity gui elements based on values in a RenderParams
 //! \param[in] rp The RenderParams instance being used.
 void updateFidelity(VAPoR::RenderParams* rp);

 void showHideVar(bool on);

 string updateVarCombo(
	QComboBox *varCombo, const vector <string> &varnames, bool doZero,
	string currentVar
 );

 void updateVariableCombos(VAPoR::RenderParams* params);

 void updateDims(VAPoR::RenderParams *rParams);

 QButtonGroup* _fidelityButtons;

 DisplayFlags _dspFlags;
 DimFlags _dimFlags;

 static string _nDimsTag;

 // Support for fidelity settings
 //
 std::vector <int> _fidelityLodIdx;
 std::vector <int> _fidelityMultiresIdx;
 std::vector <string> _fidelityLodStrs;
 std::vector <string> _fidelityMultiresStrs;

};

#endif //VARIABLESWIDGET_H 
