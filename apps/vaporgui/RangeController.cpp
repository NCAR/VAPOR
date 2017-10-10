#ifdef WIN32
#pragma warning(disable : 4100)
#endif

#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <string>
#include <cassert>
#include <cstdio>
#include "RangeController.h"

bool Observee::addObserver(Observer* observer) {
	vector<Observer*>::iterator it;
	it = find(_observers.begin(), _observers.end(), observer);
	if (it != _observers.end()) return false;

	_observers.push_back(observer);
	return true;
}

bool Observee::removeObserver(Observer* observer) {
	vector<Observer*>::iterator it;
	it = find(_observers.begin(), _observers.end(), observer);

	if (it == _observers.end()) return false;

	_observers.erase(remove(_observers.begin(), _observers.end(), observer));
	return true;
}

bool Observee::notifyObservers() {
	vector<Observer*>::iterator it;
	for (it = _observers.begin(); it!=_observers.end(); ++it) {
		(*it)->notify();
	}

    //Return false if there are no observers in the vector
    return (_observers.size() > 0);
}

void Observee::deleteObservers() {
	vector<Observer*>::iterator it;
	for (it = _observers.begin(); it!=_observers.end(); ++it) {
		delete (*it);
		(*it) = NULL;
	}
}

Range::Range(double min, double max) {
	_constant = false;
	_domainMin = min;
	_domainMax = max;

	_userMin = min;
	_userMax = max;
}

void Range::setDomainMin(double v) {
	_domainMin = v;

	if (_userMin < _domainMin) _userMin = _domainMin;

	notifyObservers();
}

void Range::setDomainMax(double v) {
	_domainMax = v;

	if (_userMax > _domainMax) _userMax = _domainMax;

	notifyObservers();
}

void Range::setUserMin(double v) {
	// Check domain bounds
	//
	if (v <= _domainMin) _userMin = _domainMin;
	else if (v >= _domainMax) _userMin = _domainMax;
	else _userMin = v;

	// If our min is less than max, adjust max
	//
	if (_userMin > _userMax) _userMax = _userMin;

	if ((_constant) && (_userMin!=_userMax)) setUserMax(v);

	cout << "Range::setUserMin " << _constant << endl;

	notifyObservers();
}

void Range::setUserMax(double v) {
	// Check domain bounds
	//
	if (v <= _domainMin) _userMax = _domainMin;
	else if (v >= _domainMax) _userMax = _domainMax;
	else _userMax = v;

	// If our max is less than min, adjust min
	//
	if (_userMax < _userMin) _userMin = _userMax;

	if ((_constant) && (_userMin!=_userMax)) setUserMin(v);

	notifyObservers();
}

void Range::setConst(bool c) {
	_constant = c;
	if (c) setUserMax(_userMin);
}

Controller::Controller(Range* range, int type) : 
	_range(range), _type(type) {
	assert(((type==0)||(type==1)) && "Controller type must be 1 or 0");
	_min = _range->getDomainMin();
	_max = _range->getDomainMax();
}

RangeLabel::RangeLabel(Range* range,
						QLabel* label,
						int type) {
	_range = range;
	_label = label;
	_type = type;

	//notify();
}

MinMaxLabel::MinMaxLabel(Range* range,
						QLabel* label,
						int type) : RangeLabel (range, label, type) {
	_range = range;
	_label = label;
	_type = type;
	notify();
}

void MinMaxLabel::notify() {
	if (_type==0) {
		_val = _range->getDomainMin();
		_label->setText(QString::number(_val));
	}
	else {
		_val = _range->getDomainMax();
		_label->setText(QString::number(_val));
	}
}

SizeLabel::SizeLabel(Range* range,
						QLabel* label,
						int type) : RangeLabel (range, label, type) {
	_range = range;
	_label = label;
	_type = type;
	notify();
}

void SizeLabel::notify() {
	if (_type==0) {
		_val = 0.f;
		_label->setText(QString::number(_val));
	}
	else {
		_val = _range->getDomainMax() - _range->getDomainMin();
		_label->setText(QString::number(_val));
	}
}

MinMaxSlider::MinMaxSlider(Range* range, 
							QSlider* slider,
							int type) : Controller(range, type) {
	_slider = slider;
	_increments = _slider->maximum() - _slider->minimum();
	_val = _type==0 ? _min : _max;

	int position = _type==0 ? 0 : _increments;
	_slider->setValue(position);

	connect(_slider, SIGNAL(valueChanged(int)), this, SLOT(updateValue()));
	//connect(_slider, SIGNAL(sliderReleased()), this, SLOT(updateValue()));
}

void MinMaxSlider::updateValue() {
	cout << "MinMaxSlider::updateValue()" << endl;
	double val;
	int pos = _slider->value();
	if (pos == _increments) val = _max;
	else if (pos == 0) val = _min;
	else val = pos * (_max-_min)/_increments + _min;

	_type==0 ? _range->setUserMin(val) : _range->setUserMax(val);
}

void MinMaxSlider::notify() {
	double min = _range->getUserMin();
	double max = _range->getUserMax();
	_min = _range->getDomainMin();
	_max = _range->getDomainMax();
	
	_val = (_type==0) ? min : max;	
	
	float denom = (_max - _min) / _increments;
	if (denom <= 0) denom = 1;

	int position = (_val-_min) / denom;

	_slider->blockSignals(true);
	_slider->setValue(position);
	_slider->blockSignals(false);
}

TimeSlider::TimeSlider(Range* range, QSlider* slider, int type) 
							: MinMaxSlider(range, slider, type) {
	_increments = range->getDomainMax() - range->getDomainMin();
	_slider->setRange(0,_increments);
	_slider->setSingleStep(1);
	
	if (type == 0) {
		_slider->setValue(range->getDomainMin());
	}
	else {
		_slider->setValue(range->getDomainMax());
	}
}

SinglePointSlider::SinglePointSlider(Range* range, 
									QSlider* slider,
									int defaultPos)
								: Controller(range) {
	_slider = slider;
	_increments = _slider->maximum() - _slider->minimum();
	_val = (_min+_max)/2.f;

	int position;
	switch(defaultPos) {
		case 0: position = 0; break;
		case 1: position = _increments; break;
		case 2: position = _increments/2; break;
		default: assert(0 && "SinglePointSlider must have a default position of 0, 1, or 2");
 	}

	_slider->setValue(position);

	connect(_slider, SIGNAL(valueChanged(int)), this, SLOT(updateValue()));
	//connect(_slider, SIGNAL(sliderReleased(int)), this, SLOT(updateValue()));
}

void SinglePointSlider::notify() {
	_min = _range->getDomainMin();
	_max = _range->getDomainMax();

	_val = _range->getUserMin();
	
	float denom = (_max - _min) / _increments;
	if (denom <= 0) denom = 1;

	_position = (_val-_min) / denom;

	_slider->blockSignals(true);
	_slider->setValue(_position);
	_slider->blockSignals(false);
}

void SinglePointSlider::updateValue() {
	double val;
	int pos = _slider->value();
	if (pos == _increments) val = _max;
	else if (pos == 0) val = _min;
	else val = pos * (_max-_min)/_increments + _min;

	_range->setUserMin(val);
	_range->setUserMax(val);
}

SinglePointLineEdit::SinglePointLineEdit(Range* range,
								QLineEdit* lineEdit,
								double defaultVal)
								: Controller(range) {
	_lineEdit = lineEdit;
	_val = defaultVal;//(_min+_max)/2.f;

	_lineEdit->setText(QString::number(_val));

	connect(_lineEdit, SIGNAL(editingFinished()), this, SLOT(updateValue()));
}

void SinglePointLineEdit::updateValue() {
	double val = _lineEdit->text().toDouble();
	_range->setUserMin(val);
	_range->setUserMax(val);
}

void SinglePointLineEdit::notify() {
	double uMin = _range->getUserMin();
	
	_lineEdit->blockSignals(true);
	_lineEdit->setText(QString::number(uMin));
	_lineEdit->blockSignals(false);
}

MinMaxLineEdit::MinMaxLineEdit(Range* range,
								QLineEdit* lineEdit,
								int type) : Controller(range, type) {
	_lineEdit = lineEdit;
	_val = type==0 ? _min : _max ;

	_lineEdit->setText(QString::number(_val));

	connect(_lineEdit, SIGNAL(editingFinished()), this, SLOT(updateValue()));
}

void MinMaxLineEdit::updateValue() {
	double val = _lineEdit->text().toDouble();
	_type==0 ? _range->setUserMin(val) : _range->setUserMax(val);
}

void MinMaxLineEdit::notify() {
	double uMin = _range->getUserMin();
	double uMax = _range->getUserMax();
	
	_val = _type==0 ? uMin : uMax;	

	_lineEdit->blockSignals(true);
	_lineEdit->setText(QString::number(_val));
	_lineEdit->blockSignals(false);
}

CenterSizeSlider::CenterSizeSlider(Range* range,
									QSlider* slider,
									int type) :
									Controller(range, type) {
	_slider = slider;
	_increments = _slider->maximum() - _slider->minimum();
	_binSize = (_max-_min)/_increments;
	_val = type==0 ? (_max+_min)/2.f : _max-_min;

	int position = type==0 ? _increments/2 : _increments;
	_slider->setValue(position);

	connect(_slider, SIGNAL(valueChanged(int)), this, SLOT(updateValue()));
	//connect(_slider, SIGNAL(sliderReleased(int)), this, SLOT(updateValue()));
}

void CenterSizeSlider::updateValue() {
	double uMin = _range->getUserMin();
	double uMax = _range->getUserMax();
	int pos = _slider->value();

	double dMin = _range->getDomainMin();
	double dMax = _range->getDomainMax();
	double binSize = (dMax - dMin)/_increments;

	double center, size;
	if (_type==0) {
		center = pos * binSize + dMin;
		size = uMax - uMin;
	}
	else if (_type==1) {
		size = pos * binSize;
		center = (uMax + uMin)/2.f;
	}
	else return;

	double newMin = center - size/2.f;
	double newMax = center + size/2.f;

	_range->setUserMin(newMin);
	_range->setUserMax(newMax);
}

void CenterSizeSlider::notify() {
	double min = _range->getUserMin();
	double max = _range->getUserMax();

	double dMin = _range->getDomainMin();
	double dMax = _range->getDomainMax();
	double binSize = (dMax - dMin)/_increments;

	int pos;
	if (_type==0) {
		double center = (max+min)/2.f;
		pos = (center - dMin) / binSize;
	}
	else if (_type==1) {
		double size = max-min;
		pos = size/binSize;
	}
	else return;

	_slider->blockSignals(true);
	_slider->setValue(pos);
	_slider->blockSignals(false);
}

CenterSizeLineEdit::CenterSizeLineEdit(Range* range,
											QLineEdit* lineEdit,
											int type) : Controller(range,type) {
	_lineEdit = lineEdit;
	_val = type==0 ? (_min+_max)/2.f : _max-_min ;

	_lineEdit->setText(QString::number(_val));

	connect(_lineEdit, SIGNAL(editingFinished()), this, SLOT(updateValue()));
}

void CenterSizeLineEdit::updateValue() {
	double dMin = _range->getDomainMin();
	double dMax = _range->getDomainMax();
	double uMin = _range->getUserMin();
	double uMax = _range->getUserMax();
	double center, size;
	double newMin, newMax;

	if (_type==0) {
		center = _lineEdit->text().toDouble();
		size = uMax - uMin;

		newMin = center - size/2.f;
		newMax = center + size/2.f;

		if (newMin < dMin) {
			newMin = dMin;
			newMax = 2*(center - dMin);
		}

		if (newMax > dMax) {
			newMax = dMax;
			newMin = dMax - 2*(dMax - center);
		}
	}
	
	else if (_type==1) {
		size = _lineEdit->text().toDouble();
		center = (uMax+uMin)/2.f;
		
		float fullSize = dMax - dMin;
		if (size > fullSize) size = fullSize;

		newMin = center - size/2.f;
		newMax = center + size/2.f;

		if (newMin < dMin) {
			newMin = dMin;
			newMax = dMin + size;
		}

		if (newMax > dMax) {
			newMax = dMax;
			newMin = dMax - size;
		}

	}
	else return;

	_range->setUserMin(newMin);
	_range->setUserMax(newMax);
}

void CenterSizeLineEdit::notify() {
	double uMin = _range->getUserMin();
	double uMax = _range->getUserMax();
	double newVal;

	newVal = _type==0 ? (uMax+uMin)/2.f : uMax-uMin;

	_lineEdit->blockSignals(true);
	_lineEdit->setText(QString::number(newVal));
	_lineEdit->blockSignals(false);
}

MinMaxTableCell::MinMaxTableCell(Range* range,
								QTableWidget* table,
								int type, int row, int col) 
								: Controller(range, type) {
	_table = table;
	_cell = table->item(row,col);
	_row = row;
	_col = col;
	_val = type==0 ? _min : _max;

	_cell->setText(QString::number(_val));

	connect(_table, SIGNAL(cellChanged(int,int)), this, SLOT(updateValue(int, int)));
}

void MinMaxTableCell::updateValue(int row, int col) {
	if (_cell != _table->item(row, col)) return;

	double val = _cell->text().toDouble();
	_type==0 ? _range->setUserMin(val) : _range->setUserMax(val);
}

void MinMaxTableCell::notify() {
	double val = _type==0 ? _range->getUserMin() : _range->getUserMax();
	QTableWidgetItem* cell = _table->item(_row,_col);

	_table->blockSignals(true);
	cell->setText(QString::number(val));
	_table->blockSignals(false);
}
