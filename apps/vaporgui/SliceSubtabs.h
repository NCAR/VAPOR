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
    void _vcbChanged( std::string value ) { cout << "vcb changed to " << value << endl; }
    void _vsbChanged( int value ) { cout << "vsb changed to " << value << endl; }
    void _vcbChanged( bool value ) { cout << "vcb changed to " << value << endl; }
    void _vleChanged( std::string value ) { cout << "vle changed to " << value << endl; }

private:
    VComboBox2* _vcb;
    VSpinBox2* _vsb;
    VCheckBox2* _vchb;
    VLineEdit2* _vle;
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
