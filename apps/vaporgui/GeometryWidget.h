#ifndef REGIONSLIDERWIDGET_H
#define REGIONSLIDERWIDGET_H

#include "ui_GeometryWidgetGUI.h"
#include "RangeCombos.h"

namespace VAPoR {
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class GeometryWidget : public QWidget, public Ui_GeometryWidgetGUI {
	
	Q_OBJECT

public:
	//! Bit mask to indicate whether 2D, 3D, or 2D and 3D variables are to be supported
	//
	enum Flags {
		TWOD = (1u << 0), 
		THREED = (1u << 1), 
		VECTOR = (1u << 2)
	};

	GeometryWidget(QWidget *parent=0);

	void Reinit(Flags flags);

	~GeometryWidget();

	QString name() const {return "GeometryWidget";}
	QString includeFile() const {return "GeometryWidget.h";}
	QString group() const {return tr("Region Control Widgets");}
	QString toolTip() const {return tr("A Region Control Widget");}
	QString whatsThis() const {return tr("This widget contains all widgets "
										"necessary for making changes to a "
										"user-defined region.");}
	bool isContainer() const {return true;}
	void Update(VAPoR::ParamsMgr* paramsMgr,
				VAPoR::DataMgr* dataMgr,
				VAPoR::RenderParams* rParams);

    bool SetUseAuxVariables( bool );    // for Statistics utility

signals:
    void valueChanged();

private slots:
	void setRange(double min, double max);
	void copyRegion();

private:
	void connectWidgets();
	void updateRangeLabels(std::vector<double> minExt,
							std::vector<double> maxExt);
	void updateCopyCombo();

	VAPoR::ParamsMgr* _paramsMgr;
	VAPoR::DataMgr* _dataMgr;
	VAPoR::RenderParams* _rParams;

	Combo* _minXCombo;
	Combo* _maxXCombo;
	RangeCombo* _xRangeCombo;

	Combo* _minYCombo;
	Combo* _maxYCombo;
	RangeCombo* _yRangeCombo;

	Combo* _minZCombo;
	Combo* _maxZCombo;
	RangeCombo* _zRangeCombo;

	std::vector <std::string> _dataSetNames;
	std::map<std::string, std::string> _visNames;
	std::map<std::string, std::string> _renTypeNames;

	Flags _flags;

    bool  _useAuxVariables;     // for Statistics utility

	static const std::string _nDimsTag;
};

#endif //REGIONSLIDERWIDGET_H
