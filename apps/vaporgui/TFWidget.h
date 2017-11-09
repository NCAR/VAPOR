#ifndef TFWIDGET_H
#define TFWIDGET_H

#include "ui_TFWidgetGUI.h"
#include "EventRouter.h"
#include "RangeCombos.h"

namespace VAPoR {
	class ControlExec;
	class MapperFunction;
}

class TFWidget : public QWidget, public Ui_TFWidgetGUI {
	
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
		SECONDARY_COLORVAR = (1u << 0),
		CONSTCOLOR = (1u << 1),

		// PRIORITY_COLORVAR just moves the color mapped variable
		// settings up higher in the gui for better visibility
		PRIORITY_COLORVAR = (1u << 2)
	};  

	TFWidget(QWidget *parent=0);

	void Reinit(Flags flags);

	~TFWidget();

	QString name() const {return "TFWidget";}
	QString includeFile() const {return "TFWidget.h";}
	QString group() const {return tr("Transfer Function Settings Widgets");}
	QString toolTip() const {return tr("A Transfer Function Settings Widget");}
	QString whatsThis() const {return tr("This widget contains all widgets "
										"necessary for making changes to a "
										"Vapor Transfer Function.");}
	bool isContainer() const {return true;}
	void Update(VAPoR::DataMgr *dataMgr,
				VAPoR::ParamsMgr *paramsMgr,
				VAPoR::RenderParams *rParams);


	void fileLoadTF(string varname, const char* path,
		bool savePath);

	void loadTF(string varname);

	void getRange(float range[2], float values[2]);

public slots:
	// Made public, to be called from EventRouter::Initialize
	//
	void setCMVar(const QString&);

private slots:
	void fileSaveTF();
	void setRange();
	void setRange(double min, double max);
	void updateHisto();
	void autoUpdateHistoChecked(int state);
	void colorInterpChanged(int index);
	void loadTF();
	void setSingleColor();
	void forwardTFChange();

private:
	void collapseAutoUpdateHistoCheckbox();
	string getVariableName();
	void setNativeTranferFunction(string var);
	void configureConstColorWidgets(string var);
	void connectWidgets();
	void updateSliders();
	void updateAutoUpdateHistoCheckbox();
	void updateColorInterpolation();
	void updateMappingFrame();
	void updateColorVarCombo();
	void enableTFWidget(bool state);
	void collapseColorVarSettings();	
	void collapseConstColorSettings();

	int confirmMinRangeEdit(VAPoR::MapperFunction* tf, float* range);
	int confirmMaxRangeEdit(VAPoR::MapperFunction* tf, float* range);

	bool _autoUpdateHisto = false;
	bool _discreteColormap = false;
	bool _textChanged = false;
	float _myRGB[3];

	//VAPoR::ControlExec* _controlExec;
	RenderEventRouter* _eventRouter;
	VAPoR::ParamsMgr* _paramsMgr;
	VAPoR::DataMgr* _dataMgr;
	VAPoR::RenderParams* _rParams;

	Combo* _minCombo = NULL;
	Combo* _maxCombo = NULL;
	RangeCombo* _rangeCombo = NULL;

	Flags _flags;

	static string _nDimsTag;

#ifdef DEAD
	void makeItRed(QLineEdit* edit);
	void makeItYellow(QLineEdit* edit);
	void makeItGreen(QLineEdit* edit);
	void makeItWhite(QLineEdit* edit);
#endif

	signals:
		void emitChange();
};

#endif //TFWIDGET_H
