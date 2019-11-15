#ifndef SLICESUBTABS_H
#define SLICESUBTABS_H

#include "ui_SliceAppearanceGUI.h"
#include "ui_SliceVariablesGUI.h"
#include "ui_SliceGeometryGUI.h"
#include "ui_SliceAnnotationGUI.h"
#include "Flags.h"

#include "VComboBox.h"
#include "VSpinBox.h"
#include "VLineEdit.h"
#include "VCheckBox.h"
#include "VSlider.h"
#include "VSliderEdit.h"
#include "VPushButton.h"
#include "VFileSelector.h"

#include <vapor/SliceParams.h>

namespace VAPoR {
	class ControlExec;
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class TFEditor;

class SliceVariablesSubtab : public QWidget, public Ui_SliceVariablesGUI {

	Q_OBJECT

public:
	SliceVariablesSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	);

private slots:
    void _setDefaultSampleRate();
    /*void _vcbChanged( std::string value ) { cout << "vcb changed to " << value << endl; }
    void _vsbChanged( int value ) { cout << "vsb changed to " << value << endl; }
    void _vcbChanged( bool value ) { _vse->SetIntType( value ); _vs->SetIntType( value ); cout << "vcb changed to " << value << endl; }
    void _vleChanged( std::string value ) { cout << "vle changed to " << value << endl; }
    void _vsChangedIntermediate( double value ) { cout << "vle intermediately changed to " << value << endl; }
    void _vsChanged( double value ) { cout << "vs changed to " << value << endl; }
    void _vseChanged( double value ) { cout << "vse changed to " << value << endl; }
    void _vseChangedIntermediate( double value ) { cout << "vse interm. changed to " << value << endl; }
    void _bChanged() { cout << "button pushed" << endl; }
    void _frChanged() { cout << "frChanged " << _fr->GetValue() << endl; } 
    void _fwChanged() { cout << "fwChanged " << _fw->GetValue() << endl; }
    void _dsChanged() { cout << "dsChanged " << _fw->GetValue() << endl; }*/
    
    void _vcbChanged( std::string value ) {};
    void _vsbChanged( int value ) {};
    void _vcbChanged( bool value ) {};
    void _vleChanged( std::string value ) {};
    void _vsChangedIntermediate( double value ) { };
    void _vsChanged( double value ) { };
    void _vseChanged( double value ) { };
    void _vseChangedIntermediate( double value ) { };
    void _bChanged() { };
    void _frChanged() { };
    void _fwChanged() { };
    void _dsChanged() { };

private:
    VComboBox* _vcb;
    VSpinBox* _vsb;
    VCheckBox* _vchb;
    VLineEdit* _vle;
    VSlider* _vs;
    VSliderEdit* _vse;
    VPushButton* _pb;
    VFileReader* _fr;
    VFileWriter* _fw;
    VDirSelector* _ds;

    VAPoR::SliceParams* _params;
};

class SliceAppearanceSubtab : public QWidget, public Ui_SliceAppearanceGUI {

	Q_OBJECT

public:
	SliceAppearanceSubtab(QWidget* parent);

	void Update(
		VAPoR::DataMgr *dataMgr,
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::RenderParams *rParams
	);

private slots:
    void _qualityChanged(int rate);

private:
    VAPoR::SliceParams* _params;
    TFEditor *_tfe;
};

class SliceGeometrySubtab : public QWidget, public Ui_SliceGeometryGUI {

	Q_OBJECT

public:
	SliceGeometrySubtab(QWidget* parent);
	
	void Update(
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::DataMgr *dataMgr,
		VAPoR::RenderParams *rParams
	); 

private:
    VAPoR::SliceParams* _params;

private slots:
    void _orientationChanged(int plane);
};

class SliceAnnotationSubtab : public QWidget, public Ui_SliceAnnotationGUI {

	Q_OBJECT

public:
	SliceAnnotationSubtab(QWidget* parent);
	
	void Update(
		VAPoR::ParamsMgr *paramsMgr,
		VAPoR::DataMgr *dataMgr,
		VAPoR::RenderParams *rParams
	);
};
#endif //SLICESUBTABS_H
