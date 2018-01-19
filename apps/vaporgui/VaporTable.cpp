//*************************************************************************
//                                                                        *
//   Copyright (C)  2017                                                  *
//   University Corporation for Atmospheric Research                      *
//   All Rights Reserved                                                  *
//                                                                        *
//************************************************************************/
//
//    File:      VaporTable.cpp
//
//    Author:  Scott Pearse
//    National Center for Atmospheric Research
//    PO 3000, Boulder, Colorado
//
//  Date:      December 2017
//

#include <QtGui>
#include <iostream>
#include <cassert>
#include <stdlib.h>
#include "VaporTable.h"

VaporTable::VaporTable(QTableWidget *table, bool lastRowIsCheckboxes, bool lastColIsCheckboxes)
{
    _table = table;
    _lastRowIsCheckboxes = lastRowIsCheckboxes;
    _lastColIsCheckboxes = lastColIsCheckboxes;
    _activeRow = -1;
    _activeCol = -1;
}

// Clear current table, then generate table of rows x columns
// Determine template type and set validators on all cells accordingly
// Determine if checkboxes are needed
// Convert values, rowHeaders, and colHeaders to QStrings, then populate
void VaporTable::Update(int rows, int cols, std::vector<int> values, std::vector<std::string> rowHeaders, std::vector<std::string> colHeaders)
{
    std::vector<std::string> sValues = convertToString(values);
    Update(rows, cols, sValues, rowHeaders, colHeaders);
}

void VaporTable::Update(int rows, int cols, std::vector<double> values, std::vector<std::string> rowHeaders, std::vector<std::string> colHeaders)
{
    std::vector<std::string> sValues = convertToString(values);
    Update(rows, cols, sValues, rowHeaders, colHeaders);
}

void VaporTable::Update(int rows, int cols, std::vector<std::string> values, std::vector<std::string> rowHeaders, std::vector<std::string> colHeaders)
{
    _table->clear();
    _table->setRowCount(rows);
    _table->setColumnCount(cols);

    setTableCells(values);
    setVerticalHeader(rowHeaders);
    setHorizontalHeader(colHeaders);

    addCheckboxes(values);

    if (_highlightFlags & ROWS) highlightActiveRow(_activeRow);
    if (_highlightFlags & COLS) highlightActiveCol(_activeCol);
}

void VaporTable::Reinit(VaporTable::ValidatorFlags vFlags, VaporTable::MutabilityFlags mFlags, VaporTable::HighlightFlags hFlags)
{
    _validatorFlags = vFlags;
    _mutabilityFlags = mFlags;
    _highlightFlags = hFlags;
}

std::vector<std::string> VaporTable::convertToString(std::vector<int> values)
{
    std::vector<std::string> sValues;

    int size = values.size();
    for (int i = 0; i < size; i++) {
        std::stringstream ss;
        ss << values[i];
        std::string s = ss.str();
        sValues.push_back(s);
    }

    return sValues;
}

std::vector<std::string> VaporTable::convertToString(std::vector<double> values)
{
    std::vector<std::string> sValues;

    int size = values.size();
    for (int i = 0; i < size; i++) {
        std::stringstream ss;
        ss << values[i];
        std::string s = ss.str();
        sValues.push_back(s);
    }

    return sValues;
}

void VaporTable::setTableCells(std::vector<std::string> values)
{
    int cols = _table->columnCount();
    int rows = _table->rowCount();

    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            int         index = i + (cols)*j;
            std::string value = values[index];

            QString qVal = QString::fromStdString(value);

            QLineEdit *edit = createLineEdit(qVal);
            edit->setProperty("row", j);
            edit->setProperty("col", i);
            _table->setCellWidget(j, i, edit);
        }
    }
}

void VaporTable::addCheckboxes(std::vector<std::string> values)
{
    int cols = _table->columnCount();
    int rows = _table->rowCount();

    if (_lastColIsCheckboxes) {
        for (int j = 0; j < rows; j++) {
            int  index = j * cols + (cols - 1);
            bool checked = isValueChecked(values, index);
            addCheckbox(j, cols - 1, checked);
        }
    }

    if (_lastRowIsCheckboxes) {
        for (int i = 0; i < cols; i++) {
            int  index = (rows - 1) * (cols - 1) + i;
            bool checked = isValueChecked(values, index);
            addCheckbox(rows - 1, i, checked);
        }
    }
}

bool VaporTable::isValueChecked(std::vector<std::string> values, int index)
{
    std::string value = values[index];

    std::stringstream ss;
    ss << value;

    bool checked = false;
    if (atoi(ss.str().c_str())) checked = true;

    return checked;
}

void VaporTable::addCheckbox(int row, int column, bool checked)
{
    _table->removeCellWidget(row, column);

    QWidget *    cbWidget = new QWidget();
    QCheckBox *  checkBox = new QCheckBox();
    QHBoxLayout *cbLayout = new QHBoxLayout(cbWidget);

    cbLayout->addWidget(checkBox);
    cbLayout->setAlignment(Qt::AlignCenter);
    cbLayout->setContentsMargins(0, 0, 0, 0);

    cbWidget->setLayout(cbLayout);

    _table->setCellWidget(row, column, cbWidget);

    if (checked) {
        checkBox->setCheckState(Qt::Checked);
    } else {
        checkBox->setCheckState(Qt::Unchecked);
    }

    // The chekbox and the widget can both be clicked,
    // so I've given both objects row and column
    // properties to relay if needed
    checkBox->setProperty("row", row);
    checkBox->setProperty("col", column);
    cbWidget->setProperty("row", row);
    cbWidget->setProperty("col", column);

    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(emitValueChanged()));

    cbWidget->installEventFilter(this);
}

QLineEdit *VaporTable::createLineEdit(QString val)
{
    QLineEdit *edit = new QLineEdit(_table);
    setValidator(edit);

    edit->setText(val);

    if (_mutabilityFlags & IMMUTABLE) {
        edit->setReadOnly(true);
    } else {
        edit->setReadOnly(false);
    }

    edit->setAlignment(Qt::AlignHCenter);

    connect(edit, SIGNAL(editingFinished()), this, SLOT(emitValueChanged()));

    edit->installEventFilter(this);

    return edit;
}

void VaporTable::emitValueChanged()
{
    QObject *obj = sender();
    int      row = obj->property("row").toInt();
    int      col = obj->property("col").toInt();

    _activeRow = row;
    _activeCol = col;
    if (_highlightFlags & ROWS) highlightActiveRow(_activeRow);
    if (_highlightFlags & COLS) highlightActiveCol(_activeCol);

    emit valueChanged(row, col);
}

void VaporTable::emitCellClicked(QObject *obj)
{
    int row = obj->property("row").toInt();
    int col = obj->property("col").toInt();

    _activeRow = row;
    _activeCol = col;
    if (_highlightFlags & ROWS) highlightActiveRow(_activeRow);
    if (_highlightFlags & COLS) highlightActiveCol(_activeCol);

    emit cellClicked(row, col);
}

bool VaporTable::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) emitCellClicked(object);
    return false;
}

void VaporTable::setValidator(QLineEdit *edit)
{
    if (_validatorFlags & INT) { edit->setValidator(new QIntValidator(edit)); }
    if (_validatorFlags & DOUBLE) { edit->setValidator(new QDoubleValidator(edit)); }
    if (_validatorFlags & STRING) {
        QRegExpValidator *validator;
        validator = new QRegExpValidator(QRegExp(".{1,64}"));
        edit->setValidator(validator);
    }
}

void VaporTable::setHorizontalHeader(std::vector<std::string> header)
{
    int size = header.size();
    if (size < 1) {
        _table->horizontalHeader()->hide();
        return;
    }

    QStringList list;
    for (int i = 0; i < size; i++) { list << QString::fromStdString(header[i]); }

    _table->resizeColumnsToContents();
    _table->setHorizontalHeaderLabels(list);
    _table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    QTableWidgetItem *headerItem;
    for (int i = 0; i < size; i++) {
        headerItem = _table->horizontalHeaderItem(i);
        if (headerItem) headerItem->setToolTip(list[i]);
    }
}

std::string VaporTable::GetHorizontalHeaderItem(int index)
{
    QString str = _table->horizontalHeaderItem(index)->text();
    return str.toStdString();
}

void VaporTable::setVerticalHeader(std::vector<std::string> header)
{
    int size = header.size();
    if (size < 1) {
        _table->verticalHeader()->hide();
        return;
    }

    QStringList list;
    for (int i = 0; i < size; i++) { list << QString::fromStdString(header[i]); }

    _table->setVerticalHeaderLabels(list);
    _table->resizeRowsToContents();
    _table->verticalHeader()->setResizeMode(QHeaderView::Stretch);

    QTableWidgetItem *headerItem;
    for (int i = 0; i < size; i++) {
        headerItem = _table->verticalHeaderItem(i);
        if (headerItem) headerItem->setToolTip(list[i]);
    }
}

std::string VaporTable::GetVerticalHeaderItem(int index)
{
    QString str = _table->verticalHeaderItem(index)->text();
    return str.toStdString();
}

void VaporTable::SetCheckboxesInFinalRow(bool enabled) { _lastRowIsCheckboxes = enabled; }

void VaporTable::SetCheckboxesInFinalColumn(bool enabled) { _lastColIsCheckboxes = enabled; }

Value VaporTable::GetValue(int row, int col)
{
    std::string value;
    int         nRows = _table->rowCount();
    int         nCols = _table->columnCount();

    QWidget *widget = _table->cellWidget(row, col);

    if ((col == nCols - 1 && _lastColIsCheckboxes) || (row == nRows - 1 && _lastRowIsCheckboxes)) {
        QCheckBox *checkBox = widget->findChild<QCheckBox *>();
        if (checkBox->isChecked())
            value = "1";
        else
            value = "0";
    } else {
        QString qvalue = ((QLineEdit *)widget)->text();
        value = qvalue.toStdString();
    }

    return {value};
}

template<typename T> std::vector<T> VaporTable::GetRow(int row)
{
    // std::vector<double> VaporTable::GetRow(int row) {
    std::string    value;
    int            nRows = _table->rowCount();
    int            nCols = _table->columnCount();
    std::vector<T> values;

    for (int col = 0; col < nCols; col++) {
        T value = GetValue(row, col);
        values.push_back(value);
    }
    return values;
}

std::string VaporTable::GetStringValue(int row, int col)
{
    std::string value;
    int         nRows = _table->rowCount();
    int         nCols = _table->columnCount();

    QWidget *widget = _table->cellWidget(row, col);

    if ((col == nCols - 1 && _lastColIsCheckboxes) || (row == nRows - 1 && _lastRowIsCheckboxes)) {
        QCheckBox *checkBox = widget->findChild<QCheckBox *>();
        if (checkBox->isChecked())
            value = "1";
        else
            value = "0";
    } else {
        QString qvalue = ((QLineEdit *)widget)->text();
        value = qvalue.toStdString();
    }

    return value;
}

void VaporTable::GetValues(std::vector<std::string> &vec)
{
    vec.clear();

    int nRows = _table->rowCount();
    int nCols = _table->columnCount();

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            std::string cellVal = GetValue(i, j);
            vec.push_back(cellVal);
        }
    }
}

void VaporTable::GetValues(std::vector<int> &vec)
{
    vec.clear();

    int nRows = _table->rowCount();
    int nCols = _table->columnCount();

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            int cellVal = GetValue(i, j);
            vec.push_back(cellVal);
        }
    }
}

void VaporTable::GetValues(std::vector<double> &vec)
{
    vec.clear();

    int nRows = _table->rowCount();
    int nCols = _table->columnCount();

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            double cellVal = GetValue(i, j);
            vec.push_back(cellVal);
        }
    }
}

void VaporTable::highlightActiveRow(int row)
{
    if (row < 0) return;

    QString selectionColor = "{ background: rgb(0, 255, 255);"
                             " selection-background-color: rgb(233, 99, 0); }";
    QString normalColor = "{ background: rgb(255,255,255);"
                          " selection-background-color: rgb(233, 99, 0); }";

    for (int i = 0; i < _table->rowCount(); i++) {
        for (int j = 0; j < _table->columnCount(); j++) {
            QWidget *  cell = _table->cellWidget(i, j);
            QLineEdit *le = qobject_cast<QLineEdit *>(cell);
            if (le) {
                if (i == row)
                    le->setStyleSheet("QLineEdit " + selectionColor);
                else
                    le->setStyleSheet("QLineEdit " + normalColor);
            } else {
                if (i == row)
                    cell->setStyleSheet("QWidget " + selectionColor);
                else
                    cell->setStyleSheet("QWidget " + normalColor);
            }
        }
    }
}

void VaporTable::highlightActiveCol(int col)
{
    if (col < 0) return;

    QString selectionColor = "{ background: rgb(0, 255, 255);"
                             " selection-background-color: rgb(233, 99, 0); }";
    QString normalColor = "{ background: rgb(255,255,255);"
                          " selection-background-color: rgb(233, 99, 0); }";

    for (int i = 0; i < _table->rowCount(); i++) {
        for (int j = 0; j < _table->columnCount(); j++) {
            QWidget *  cell = _table->cellWidget(i, j);
            QLineEdit *le = qobject_cast<QLineEdit *>(cell);
            if (le) {
                if (j == col)
                    le->setStyleSheet("QLineEdit " + selectionColor);
                else
                    le->setStyleSheet("QLineEdit " + normalColor);
            } else {
                if (j == col)
                    cell->setStyleSheet("QWidget " + selectionColor);
                else
                    cell->setStyleSheet("QWidget " + normalColor);
            }
        }
    }
}
