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
#include <QLineEdit>
#include "TransformTable.h"
#include "vapor/RenderParams.h"
#include "vapor/Transform.h"
#include "vapor/ViewpointParams.h"
#include "MainForm.h"
#include <typeinfo>

using namespace VAPoR;

TransformTable::TransformTable(QWidget *parent)
{
    setupUi(this);

    _scaleTable = new VaporTable(scaleTable);
    _scaleTable->Reinit((VaporTable::ValidatorFlags)(VaporTable::DOUBLE), (VaporTable::MutabilityFlags)(VaporTable::MUTABLE), (VaporTable::HighlightFlags)(0));
    connect(_scaleTable, SIGNAL(valueChanged(int, int)), this, SLOT(ScaleChanged(int, int)));

    _translationTable = new VaporTable(translationTable);
    _translationTable->Reinit((VaporTable::ValidatorFlags)(VaporTable::DOUBLE), (VaporTable::MutabilityFlags)(VaporTable::MUTABLE), (VaporTable::HighlightFlags)(0));
    connect(_translationTable, SIGNAL(valueChanged(int, int)), this, SLOT(TranslationChanged(int, int)));

    _rotationTable = new VaporTable(rotationTable);
    _rotationTable->Reinit((VaporTable::ValidatorFlags)(VaporTable::DOUBLE), (VaporTable::MutabilityFlags)(VaporTable::MUTABLE), (VaporTable::HighlightFlags)(0));
    connect(_rotationTable, SIGNAL(valueChanged(int, int)), this, SLOT(RotationChanged(int, int)));

    _originTable = new VaporTable(originTable);
    _originTable->Reinit((VaporTable::ValidatorFlags)(VaporTable::DOUBLE), (VaporTable::MutabilityFlags)(VaporTable::MUTABLE), (VaporTable::HighlightFlags)(0));
    connect(_originTable, SIGNAL(valueChanged(int, int)), this, SLOT(OriginChanged(int, int)));

    _horizontalHeaders.push_back("X");
    _horizontalHeaders.push_back("Y");
    _horizontalHeaders.push_back("Z");
}

void TransformTable::Update(const std::map<string, Transform *> &transforms)
{
    _transforms = transforms;

    _verticalHeaders.clear();
    int                                     numDatasets = transforms.size();
    map<std::string, Transform *>::iterator it;
    for (it = _transforms.begin(); it != _transforms.end(); ++it) {
        string datasetName = it->first;
        if (datasetName == "") datasetName = " ";
        _verticalHeaders.push_back(datasetName);
    }

    UpdateScales();
    UpdateTranslations();
    UpdateRotations();
    UpdateOrigins();
}

void TransformTable::UpdateScales()
{
    int                                     numDatasets = 0;
    std::vector<double>                     allScales;
    std::vector<double>                     transformScales;
    map<std::string, Transform *>::iterator it;
    for (it = _transforms.begin(); it != _transforms.end(); it++) {
        numDatasets++;
        Transform *t = it->second;
        transformScales = t->GetScales();
        for (int i = 0; i < transformScales.size(); i++) { allScales.push_back(transformScales[i]); }
    }

    if (_verticalHeaders.size()) _scaleTable->Update(numDatasets, 3, allScales, _verticalHeaders, _horizontalHeaders);
}

void TransformTable::UpdateTranslations()
{
    int                                     numDatasets = 0;
    std::vector<double>                     allTranslations;
    std::vector<double>                     transformTranslations;
    map<std::string, Transform *>::iterator it;
    for (it = _transforms.begin(); it != _transforms.end(); it++) {
        numDatasets++;
        Transform *t = it->second;
        transformTranslations = t->GetTranslations();
        for (int i = 0; i < transformTranslations.size(); i++) { allTranslations.push_back(transformTranslations[i]); }
    }

    _translationTable->Update(numDatasets, 3, allTranslations, _verticalHeaders, _horizontalHeaders);
}

void TransformTable::UpdateRotations()
{
    int                                     numDatasets = 0;
    std::vector<double>                     allRotations;
    std::vector<double>                     transformRotations;
    map<std::string, Transform *>::iterator it;
    for (it = _transforms.begin(); it != _transforms.end(); it++) {
        numDatasets++;
        Transform *t = it->second;
        transformRotations = t->GetRotations();
        for (int i = 0; i < transformRotations.size(); i++) { allRotations.push_back(transformRotations[i]); }
    }

    _rotationTable->Update(numDatasets, 3, allRotations, _verticalHeaders, _horizontalHeaders);
}

void TransformTable::UpdateOrigins()
{
    int                                     numDatasets = 0;
    std::vector<double>                     allOrigins;
    std::vector<double>                     transformOrigins;
    map<std::string, Transform *>::iterator it;
    for (it = _transforms.begin(); it != _transforms.end(); it++) {
        numDatasets++;
        Transform *t = it->second;
        transformOrigins = t->GetOrigin();
        for (int i = 0; i < transformOrigins.size(); i++) { allOrigins.push_back(transformOrigins[i]); }
    }

    _originTable->Update(numDatasets, 3, allOrigins, _verticalHeaders, _horizontalHeaders);
}

void TransformTable::ScaleChanged(int row, int col)
{
    double xScale = _scaleTable->GetValue(row, 0);
    double yScale = _scaleTable->GetValue(row, 1);
    double zScale = _scaleTable->GetValue(row, 2);

    std::vector<double> scales;
    scales.push_back(xScale);
    scales.push_back(yScale);
    scales.push_back(zScale);

    string target = _scaleTable->GetVerticalHeaderItem(row);

    map<string, Transform *>::const_iterator itr;
    itr = _transforms.find(target);
    if (itr == _transforms.end()) return;

    Transform *t = itr->second;
    t->SetScales(scales);
}

void TransformTable::TranslationChanged(int row, int col)
{
    double xTranslation = _translationTable->GetValue(row, 0);
    double yTranslation = _translationTable->GetValue(row, 1);
    double zTranslation = _translationTable->GetValue(row, 2);

    std::vector<double> translations;
    translations.push_back(xTranslation);
    translations.push_back(yTranslation);
    translations.push_back(zTranslation);

    string target = _translationTable->GetVerticalHeaderItem(row);

    map<string, Transform *>::const_iterator itr;
    itr = _transforms.find(target);
    if (itr == _transforms.end()) return;

    Transform *t = itr->second;
    t->SetTranslations(translations);
}

void TransformTable::RotationChanged(int row, int col)
{
    double xRotation = _rotationTable->GetValue(row, 0);
    double yRotation = _rotationTable->GetValue(row, 1);
    double zRotation = _rotationTable->GetValue(row, 2);

    std::vector<double> rotations;
    rotations.push_back(xRotation);
    rotations.push_back(yRotation);
    rotations.push_back(zRotation);

    string target = _rotationTable->GetVerticalHeaderItem(row);

    map<string, Transform *>::const_iterator itr;
    itr = _transforms.find(target);
    if (itr == _transforms.end()) return;

    Transform *t = itr->second;
    t->SetRotations(rotations);
}

void TransformTable::OriginChanged(int row, int col)
{
    double xOrigin = _originTable->GetValue(row, 0);
    double yOrigin = _originTable->GetValue(row, 1);
    double zOrigin = _originTable->GetValue(row, 2);

    std::vector<double> origins;
    origins.push_back(xOrigin);
    origins.push_back(yOrigin);
    origins.push_back(zOrigin);

    string target = _originTable->GetVerticalHeaderItem(row);

    map<string, Transform *>::const_iterator itr;
    itr = _transforms.find(target);
    if (itr == _transforms.end()) return;

    Transform *t = itr->second;
    t->SetOrigin(origins);
}
