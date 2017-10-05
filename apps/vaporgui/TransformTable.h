#ifndef TRANSFORMTABLE_H
#define TRANSFORMTABLE_H


#include <QObject>
#include "vapor/MyBase.h"
#include "ui_TransformTableGUI.h"

QT_USE_NAMESPACE



namespace VAPoR {
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class RenderEventRouter;

//!
//! \class TransformTable
//! \ingroup Public_GUI
//! \brief A reusable promoted widget for transforming different data sets
//! \author Scott Pearse
//! \version 3.0
//! \date  September 2017

class TransformTable : public QWidget, public Ui_TransformTableGUI {

	Q_OBJECT

public: 

 TransformTable(QWidget* parent) {
	setupUi(this);
	scaleTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	translationTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	rotationTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	
	scaleTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	rotationTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	translationTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
 };

 void Reinit() {};

 virtual ~TransformTable(){}

 virtual void Update(
	const VAPoR::DataMgr *dataMgr,
	VAPoR::ParamsMgr *paramsMgr,
	VAPoR::RenderParams *rParams
 ) {};

protected slots:

private:
 const VAPoR::DataMgr *_dataMgr;
 VAPoR::ParamsMgr *_paramsMgr;
 VAPoR::RenderParams *_rParams;
 
};

#endif //TRANSFORMTABLE_H 
