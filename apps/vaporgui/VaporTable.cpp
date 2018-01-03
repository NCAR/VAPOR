#include <QtGui>
#include <iostream>
#include <cassert>
#include <stdlib.h>

#include "VaporTable.h"

//////////////////////////////////////////////////////
//
// VaporTable
//
//////////////////////////////////////////////////////

VaporTable::VaporTable(QTableWidget *table, bool lastRowIsCheckboxes, bool lastColIsCheckboxes)
{
    _table = table;
    _lastRowIsCheckboxes = lastRowIsCheckboxes;
    _lastColIsCheckboxes = lastColIsCheckboxes;
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
}

void VaporTable::Reinit(VaporTable::ValidatorFlags vFlags, VaporTable::MutabilityFlags mFlags)
{
    _validatorFlags = vFlags;
    _mutabilityFlags = mFlags;
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
            int  index = j * (cols - 1) + (cols - 1);
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
    emit     valueChanged(row, col);
}

void VaporTable::emitCellClicked(QObject *obj)
{
    int  row = obj->property("row").toInt();
    int  col = obj->property("col").toInt();
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
        // validator = new QRegExpValidator(QRegExp("[a-zA-Z0-9]{1-64}"));
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
}

void VaporTable::setCheckboxesInFinalRow(bool enabled) { _lastRowIsCheckboxes = enabled; }

void VaporTable::setCheckboxesInFinalColumn(bool enabled) { _lastColIsCheckboxes = enabled; }

// void valueChanged(int row, int column, std::string &value);
// void valueChanged(int row, int column, double &value);
// void valueChanged(int row, int column, int &value);

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
