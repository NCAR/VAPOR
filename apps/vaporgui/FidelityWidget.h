#ifndef FIDELITYWIDGET_H
#define FIDELITYWIDGET_H


#include <QObject>
#include "vapor/MyBase.h"
#include "ui_FidelityWidgetGUI.h"

QT_USE_NAMESPACE



namespace VAPoR {
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class RenderEventRouter;

//!
//! \class FidelityWidget
//! \ingroup Public_GUI
//! \brief A Widget that can be reused to provide fidelity 
//! selection in any renderer EventRouter class
//! \author Scott Pearse
//! \version 3.0
//! \date  December 2017

class FidelityWidget : public QWidget, public Ui_FidelityWidgetGUI {


	Q_OBJECT

public: 
 enum DisplayFlags {
	SCALAR = (1u << 0), 
	VECTOR = (1u << 1), 
	COLOR = (1u << 2),
 	AUXILLARY = (1u << 3),
	HEIGHT = (1u << 4),
	};

 FidelityWidget(QWidget* parent);


 void Reinit(DisplayFlags dspFlags) {
	_dspFlags = dspFlags;
 }

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

private:
 DisplayFlags _dspFlags;
 const VAPoR::DataMgr *_dataMgr;
 VAPoR::ParamsMgr *_paramsMgr;
 VAPoR::RenderParams *_rParams;
 
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
 );

 //! Set the fidelity gui elements based on values in a RenderParams
 void updateFidelity();

 QButtonGroup* _fidelityButtons;

 // Support for fidelity settings
 //
 std::vector <int> _fidelityLodIdx;
 std::vector <int> _fidelityMultiresIdx;
 std::vector <string> _fidelityLodStrs;
 std::vector <string> _fidelityMultiresStrs;

};

#endif //FIDELITYWIDGET_H 
