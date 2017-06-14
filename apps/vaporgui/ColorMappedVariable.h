#ifndef COLORMAPPEDVARIABLE_H
#define COLORMAPPEDVARIABLE_H

#include "ColorMappedVariableGUI.h"
#include "EventRouter.h"
#include "RangeCombos.h"

namespace VAPoR {
	class ControlExec;
	class MapperFunction;
}

class ColorMappedVariable : public QWidget, public Ui_ColorMappedVariableGUI {
	
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
	enum Flags {
		COLORMAPPED = (1u << 0), 
	};  

	ColorMappedVariable(QWidget *parent=0);

	void Reinit(Flags flags);

	~ColorMappedVariable();

	QString name() const {return "ColorMappedVariable";}
	QString includeFile() const {return "ColorMappedVariable.h";}
	QString group() const {return tr("Color Map Variable Widgets");}
	QString toolTip() const {return tr("A Color Map Variable Widget");}
	QString whatsThis() const {return tr("This widget contains all widgets "
										"necessary for making changes to "
										"color mapped variables.");}
	bool isContainer() const {return true;}
	void Update(VAPoR::ParamsMgr *paramsMgr,
				VAPoR::DataMgr *dataMgr,
				VAPoR::RenderParams *rParams);

public slots:
	void setCMVar(); 
	void setSingleColor();

private:
	
	void connectWidgets();
	VAPoR::RenderParams* _rParams;

	Flags _flags;

	float _myRGB[3];
};

#endif //COLORMAPPEDVARIABLE_H
