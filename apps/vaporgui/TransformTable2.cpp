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

TransformTable::TransformTable(QWidget* parent) {
    setupUi(this);

	_scaleTable = new VaporTable(scaleTable);
	_translationTable = new VaporTable(translationTable);
	_rotationTable = new VaporTable(rotationTable);
	_originTable = new VaporTable(originTable);

	_horizontalHeaders.push_back("Target");
	_horizontalHeaders.push_back("X");
	_horizontalHeaders.push_back("Y");
	_horizontalHeaders.push_back("Z");

    /*scaleTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    translationTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rotationTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    originTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        
    scaleTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    rotationTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    translationTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    originTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);*/
}

void TransformTable::Update(const std::map <string, Transform *> &transforms) {

	_transforms = transforms;

	int numDatasets = transforms.size();
	for (int i=0; i<numDatasets; i++) {
		_verticalHeaders.push_Back(
	}

	updateScales();

//	_translationTable->Update(
//		numDatasets, 4, translations, _verticalHeaders, _horizontalHeaders
//	);
//	_rotationTable->Update(
//		numDatasets, 4, rotations, _verticalHeaders, _horizontalHeaders
//	);	
//	_originTable->Update(
//		numDatasets, 4, origins, _verticalHeaders, _horizontalHeaders
//	);
	/*updateScales();
	updateTranslations();
	updateRotations();
	updateOrigin();*/
}

void TransformTable::updateTransformTable(QTableWidget* table,
    string target, vector<double> values, int row) {

    table->blockSignals(true);

	QLineEdit* item;

	item = new QLineEdit(table);
	item->setText(QString::fromStdString(target));
    item->setAlignment(Qt::AlignCenter);
	item->setReadOnly(true);
    table->setCellWidget(row, 0, item);

	item = new QLineEdit(table);
	item->setText(QString::number(values[0]));
    item->setValidator(new QDoubleValidator(item));
    item->setAlignment(Qt::AlignCenter);
	item->setProperty("row", row);
	item->setProperty("col", 1);
	connect(item, SIGNAL(editingFinished()), this,
		SLOT(transformChanged()));
    table->setCellWidget(row, 1, item);


	item = new QLineEdit(table);
	item->setText(QString::number(values[1]));
    item->setValidator(new QDoubleValidator(item));
    item->setAlignment(Qt::AlignCenter);
	item->setProperty("row", row);
	item->setProperty("col", 2);
	connect(item, SIGNAL(editingFinished()), this,
		SLOT(transformChanged()));
    table->setCellWidget(row, 2, item);

	item = new QLineEdit(table);
	item->setText(QString::number(values[2]));
    item->setValidator(new QDoubleValidator(item));
    item->setAlignment(Qt::AlignCenter);
	item->setProperty("row", row);
	item->setProperty("col", 3);
	connect(item, SIGNAL(editingFinished()), this,
		SLOT(transformChanged()));
    table->setCellWidget(row, 3, item);

    QHeaderView* header = table->verticalHeader();
    header->setResizeMode(QHeaderView::Stretch);
    header->hide();

    table->blockSignals(false);
}


void TransformTable::updateScales() {
	std::vector<double> allScales;
	std::vector<double> transformScales;
	map<std::string, Transform*>::iterator it;
	for (it = _transforms.begin(); it != _transforms.end(); it++) {
		Tramsform* t = it->first;
		transformScales = t->GetScales();
		for (int i=0; i<transformScales.size(); i++) {
			allScales.push_back(transformScales[i]);
		}
	}
	
	_scaleTable->Update(
		numDatasets, 4, allScales, _verticalHeaders, _horizontalHeaders
	);
}

void TransformTable::updateTranslations() {
    QTableWidget* table = translationTable;

    table->setRowCount(_transforms.size());

	std::map <string, Transform *>::const_iterator itr;
	int row = 0;
	for (itr = _transforms.cbegin(); itr != _transforms.cend(); ++itr) {
		string target = itr->first;
		const Transform *t = itr->second;

        updateTransformTable(table, target, t->GetTranslations(), row);
		row++;
    }
}

void TransformTable::updateRotations() {
    QTableWidget* table = rotationTable;

    table->setRowCount(_transforms.size());

	std::map <string, Transform *>::const_iterator itr;
	int row = 0;
	for (itr = _transforms.cbegin(); itr != _transforms.cend(); ++itr) {
		string target = itr->first;
		const Transform *t = itr->second;

        updateTransformTable(table, target, t->GetRotations(), row);
		row++;
    }
}

void TransformTable::updateOrigin() {
    QTableWidget* table = originTable;

    table->setRowCount(_transforms.size());

	std::map <string, Transform *>::const_iterator itr;
	int row = 0;
	for (itr = _transforms.cbegin(); itr != _transforms.cend(); ++itr) {
		string target = itr->first;
		const Transform *t = itr->second;

        updateTransformTable(table, target, t->GetOrigin(), row);
		row++;
    }
}

void TransformTable::setScales(string target, vector<double> scale) {

	map <string, Transform *>::const_iterator itr;
	itr = _transforms.find(target);
	if (itr == _transforms.end()) return;

	Transform *t = itr->second;
	t->SetScales(scale);
}

void TransformTable::setOrigin(string target, vector<double> origin) {

	map <string, Transform *>::const_iterator itr;
	itr = _transforms.find(target);
	if (itr == _transforms.end()) return;

	Transform *t = itr->second;
	t->SetOrigin(origin);
}

void TransformTable::setRotations(
	string target, 
	vector<double> rotation
) {
	map <string, Transform *>::const_iterator itr;
	itr = _transforms.find(target);
	if (itr == _transforms.end()) return;

	Transform *t = itr->second;
	t->SetRotations(rotation);
}

void TransformTable::transformChanged() {
	QLineEdit* le = (QLineEdit*)sender();
	QTableWidget* table = (QTableWidget*)(le->parentWidget()->parentWidget());
	int row = sender()->property("row").toInt();
	int col = sender()->property("col").toInt();
	transformChanged(table, row, col);
}

void TransformTable::transformChanged(QTableWidget* table, 
	int row, int col
	) {
	vector<double> translation;
	QLineEdit* le;
	le = (QLineEdit*)table->cellWidget(row, 0);
	string target = le->text().toStdString();

	le = (QLineEdit*)table->cellWidget(row,1);
	double x = le->text().toDouble();
	
	le = (QLineEdit*)table->cellWidget(row,2);
	double y = le->text().toDouble();
	
	le = (QLineEdit*)table->cellWidget(row,3);
	double z = le->text().toDouble();

	translation.push_back(x);
	translation.push_back(y);
	translation.push_back(z);

	if (table->objectName() == "translationTable")
		setTranslations(target, translation);
	else if (table->objectName() == "scaleTable")
		setScales(target, translation);
	else if (table->objectName() == "rotationTable")
		setRotations(target, translation);
	else if (table->objectName() == "originTable")
		setOrigin(target, translation);
}

void TransformTable::setTranslations(
	string target,
	vector<double> translation
) {
	map <string, Transform *>::const_iterator itr;
	itr = _transforms.find(target);
	if (itr == _transforms.end()) return;

	Transform *t = itr->second;
	t->SetTranslations(translation);
}

