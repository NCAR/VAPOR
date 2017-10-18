//************************************************************************
//							  *
//	   Copyright (C)  2017					*
//	 University Corporation for Atmospheric Research		  *
//	   All Rights Reserved					*
//							  *
//************************************************************************/
//
//  File:	   TransformTable.cpp
//
//  Author:	 Scott Pearse
//	  National Center for Atmospheric Research
//	  PO 3000, Boulder, Colorado
//
//  Date:	   October 2017
//
//  Description:	Implements the TransformTable class.  This provides
//  a widget that is inserted in the "Geometry" tab of various Renderer GUIs
//
#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include "TransformTable.h"
#include "vapor/RenderParams.h"
#include "vapor/ViewpointParams.h"
#include "MainForm.h"

TransformTable::TransformTable(QWidget *parent)
{
    setupUi(this);
    scaleTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    translationTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rotationTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scaleTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    rotationTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    translationTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    connect(scaleTable, SIGNAL(cellChanged(int, int)), this, SLOT(scaleChanged(int, int)));
    connect(rotationTable, SIGNAL(cellChanged(int, int)), this, SLOT(rotationChanged(int, int)));
    connect(translationTable, SIGNAL(cellChanged(int, int)), this, SLOT(translationChanged(int, int)));

    _controlExec = NULL;
    _rParams = NULL;
}

void TransformTable::Update(VAPoR::RenderParams *rParams)
{
    // This function should be called from within a RenderEventRouter
    // or its subtabs
    //
    assert(_flags && RENDERER);
    _rParams = rParams;

    cout << "Updating TransformTable for renderer" << endl;

    updateRendererScales();
    updateRendererTranslations();
    updateRendererRotations();
}

void TransformTable::Update(VAPoR::ControlExec *controlExec)
{
    // This function should be called within the ViewpointEventRouter
    //
    assert(_flags && VIEWPOINT);
    _controlExec = controlExec;

    updateViewpointScales();
    updateViewpointRotations();
    updateViewpointTranslations();
}

void TransformTable::updateTransformTable(QTableWidget *table, string target, vector<double> values, int row)
{
    table->blockSignals(true);

    QTableWidgetItem *item;

    item = new QTableWidgetItem(QString::fromStdString(target));
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    item->setTextAlignment(Qt::AlignCenter);
    table->setItem(row, 0, item);

    item = new QTableWidgetItem(QString::number(values[0]));
    item->setTextAlignment(Qt::AlignCenter);
    table->setItem(row, 1, item);

    item = new QTableWidgetItem(QString::number(values[1]));
    item->setTextAlignment(Qt::AlignCenter);
    table->setItem(row, 2, item);

    item = new QTableWidgetItem(QString::number(values[2]));
    item->setTextAlignment(Qt::AlignCenter);
    table->setItem(row, 3, item);

    QHeaderView *header = table->verticalHeader();
    header->setResizeMode(QHeaderView::Stretch);
    header->hide();

    table->blockSignals(false);
}

void TransformTable::updateViewpointScales()
{
    QTableWidget *table = scaleTable;

    vector<double> sFactors;

    VAPoR::ParamsMgr *pm = _controlExec->GetParamsMgr();
    vector<string>    winNames = _controlExec->GetVisualizerNames();

    VAPoR::ViewpointParams *vpp;
    vpp = pm->GetViewpointParams(winNames[0]);

    vector<string> datasetNames = _controlExec->getDataStatus()->GetDataMgrNames();
    table->setRowCount(datasetNames.size());

    for (int i = 0; i < datasetNames.size(); i++) {
        sFactors = vpp->GetScales(datasetNames[i]);
        updateTransformTable(table, datasetNames[i], sFactors, i);
    }
}

void TransformTable::updateViewpointTranslations()
{
    QTableWidget *table = translationTable;

    vector<double> translations;

    VAPoR::ParamsMgr *pm = _controlExec->GetParamsMgr();
    vector<string>    winNames = _controlExec->GetVisualizerNames();

    VAPoR::ViewpointParams *vpp;
    vpp = pm->GetViewpointParams(winNames[0]);

    vector<string> datasetNames = _controlExec->getDataStatus()->GetDataMgrNames();
    table->setRowCount(datasetNames.size());

    for (int i = 0; i < datasetNames.size(); i++) {
        translations = vpp->GetTranslations(datasetNames[i]);
        updateTransformTable(table, datasetNames[i], translations, i);
    }
}

void TransformTable::updateViewpointRotations()
{
    QTableWidget *table = rotationTable;

    vector<double> rotations;

    VAPoR::ParamsMgr *pm = _controlExec->GetParamsMgr();
    vector<string>    winNames = _controlExec->GetVisualizerNames();

    VAPoR::ViewpointParams *vpp;
    vpp = pm->GetViewpointParams(winNames[0]);

    vector<string> datasetNames = _controlExec->getDataStatus()->GetDataMgrNames();
    table->setRowCount(datasetNames.size());

    for (int i = 0; i < datasetNames.size(); i++) {
        rotations = vpp->GetRotations(datasetNames[i]);
        updateTransformTable(table, datasetNames[i], rotations, i);
    }
}

void TransformTable::scaleChanged(int row, int col)
{
    vector<double> scale;
    QTableWidget * table = scaleTable;
    string         dataset = table->item(row, 0)->text().toStdString();
    double         x = table->item(row, 1)->text().toDouble();
    double         y = table->item(row, 2)->text().toDouble();
    double         z = table->item(row, 3)->text().toDouble();
    scale.push_back(x);
    scale.push_back(y);
    scale.push_back(z);

    if (_flags && VIEWPOINT) { setViewpointScales(dataset, scale); }
    if (_flags && RENDERER) { setRendererScales(scale); }
}

void TransformTable::setViewpointScales(string dataset, vector<double> scale)
{
    VAPoR::ParamsMgr *pm = _controlExec->GetParamsMgr();
    vector<string>    winNames = _controlExec->GetVisualizerNames();

    VAPoR::ViewpointParams *vpp;
    for (int i = 0; i < winNames.size(); i++) {
        vpp = pm->GetViewpointParams(winNames[i]);
        vpp->SetScales(dataset, scale);
    }
}

void TransformTable::updateRendererScales() {}

void TransformTable::setRendererScales(vector<double> scale)
{
    //_rParams->setScale(scale);
}

void TransformTable::rotationChanged(int row, int col)
{
    vector<double> rotation;
    QTableWidget * table = rotationTable;
    string         dataset = table->item(row, 0)->text().toStdString();
    double         x = table->item(row, 1)->text().toDouble();
    double         y = table->item(row, 2)->text().toDouble();
    double         z = table->item(row, 3)->text().toDouble();
    rotation.push_back(x);
    rotation.push_back(y);
    rotation.push_back(z);

    if (_flags && VIEWPOINT) { setViewpointRotations(dataset, rotation); }
    if (_flags && RENDERER) { setRendererRotations(rotation); }
}

void TransformTable::setViewpointRotations(string dataset, vector<double> rotation)
{
    VAPoR::ParamsMgr *pm = _controlExec->GetParamsMgr();
    vector<string>    winNames = _controlExec->GetVisualizerNames();

    VAPoR::ViewpointParams *vpp;
    for (int i = 0; i < winNames.size(); i++) {
        vpp = pm->GetViewpointParams(winNames[i]);
        vpp->SetRotations(dataset, rotation);
    }
}

void TransformTable::updateRendererRotations() {}

void TransformTable::setRendererRotations(vector<double> rotation)
{
    //_rParams->setRotations(scale);
}

void TransformTable::translationChanged(int row, int col)
{
    vector<double> translation;
    QTableWidget * table = translationTable;
    string         dataset = table->item(row, 0)->text().toStdString();
    double         x = table->item(row, 1)->text().toDouble();
    double         y = table->item(row, 2)->text().toDouble();
    double         z = table->item(row, 3)->text().toDouble();
    translation.push_back(x);
    translation.push_back(y);
    translation.push_back(z);

    if (_flags && VIEWPOINT) { setViewpointTranslations(dataset, translation); }
    if (_flags && RENDERER) { setRendererTranslations(translation); }
}

void TransformTable::setViewpointTranslations(string dataset, vector<double> translation)
{
    VAPoR::ParamsMgr *pm = _controlExec->GetParamsMgr();
    vector<string>    winNames = _controlExec->GetVisualizerNames();

    VAPoR::ViewpointParams *vpp;
    for (int i = 0; i < winNames.size(); i++) {
        vpp = pm->GetViewpointParams(winNames[0]);
        vpp->SetTranslations(dataset, translation);
    }
}

void TransformTable::updateRendererTranslations() {}

void TransformTable::setRendererTranslations(vector<double> t)
{
    //_rParams->setTranslations(t);
}
