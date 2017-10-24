#include <QtGui>
#include <iostream>
#include <cassert>

using namespace std;

#include "RangeCombos.h"


//////////////////////////////////////////////////////
//
// Combo
//
//////////////////////////////////////////////////////

Combo::Combo(QLineEdit* edit, QSlider* slider, bool intType)
{

	_minValid = 0.0;
	_maxValid = 1.0;
	_value = _minValid;
	_intType = intType;

    _floatPrecision = 6;   // 6 is default in QT

	_lineEdit = NULL;
	_lineEditValidator = NULL;
	_slider = NULL;

	_lineEdit = edit;

	if (_intType) {
		_lineEditValidator = new QIntValidator(_lineEdit);
		_lineEdit->setValidator((QIntValidator *) _lineEditValidator);
	}
	else {
		_lineEditValidator = new QDoubleValidator(_lineEdit);
		_lineEdit->setValidator((QDoubleValidator *) _lineEditValidator);
	} 


	// Set up slot for changes to _lineEdit
	//
	connect(
		_lineEdit, SIGNAL(returnPressed()),
		this, SLOT(setLineEdit())
	);

	_slider= slider;
	_slider->setFocusPolicy(Qt::StrongFocus);
	_slider->setSingleStep(1);
	_slider->setMinimum(0);
	_slider->setMaximum(999);
	_slider->setTracking(true);	// Get bizzare double sliders without this

	// Set up slot for changes to _slider
	//
	connect(
		_slider, SIGNAL(sliderReleased()),
		this, SLOT(setSlider())
	);

    // update values displayed in the _lineEdit without incurring any state upate.
    connect( _slider, SIGNAL(sliderMoved(int)), this, SLOT(setSliderMini( int )) ); 

	// This is the aggregate slot that is fired when either the _slider
	// or the _lineEdit  object change value
	//
	connect(
			this, SIGNAL(valueChanged(double)),
			this, SLOT(SetSliderLineEdit(double))
	);
}

void Combo::Update(double min, double max, double value) {

	// Make sure all parameters are valid. If not make them valid
	//
	// Should we issue a signal to notify the change of input
	// parameters?
	//
	if (min >= max) max = min; 
	if (value < min) value = min;
	if (value > max) value = max;

	_minValid = min;
	_maxValid = max;
	_value = value;

	// Update validator
	//
	// Note: The QDoubleValidator has unexpected behavior and permits
	// values outside of the the valid range. Possible to options to correct
	// this are:
	// 
	// 1. Subclass QDoubleValidator to perform the expected behavior
	// 2. Use QDoubleValidator only to ensure that input is a double,
	// and perform range checking inside the slot for returnPressed()
	//
	// Currently we use option (2)

	// Update the GUI to reflect the new values
	//
	bool oldState = _lineEdit->blockSignals(true);
	if (_intType) {
		_lineEdit->setText(QString::number((int) value));
	}
	else {
		_lineEdit->setText(QString::number( value, 'g', _floatPrecision ));
	}
	_lineEdit->blockSignals(oldState);

	int pos = 0;
	if (_minValid != _maxValid) {
		pos = (int) (((value - _minValid) / (_maxValid - _minValid)) *
			(_slider->maximum() - _slider->minimum()) + _slider->minimum());
	}

	oldState = _slider->blockSignals(true);
	_slider->setSliderPosition(pos);
	_slider->blockSignals(oldState);
	
}

void Combo::SetEnabled(bool on) {
     _lineEdit->setEnabled(on);
     _slider->setEnabled(on);
}

void Combo::setLineEdit() {
	double value = _lineEdit->text().toDouble();

	// The Qt QDoubleValidator class doesn't work as expected. Values
	// outside of valid range (set with setTop() and setBottom()) do not
	// trigger the returnPressed() slot, but the QLineEdit will still
	// display the out-of-range value. Below, we force out of range
	// values to be in range, and update the GUI to reflect the valid value
	//
	if (value < _minValid) {
		value = _minValid;
		if (_intType) {
			_lineEdit->setText(QString::number((int) value));
		}
		else {
			_lineEdit->setText(QString::number( value, 'g', _floatPrecision ));
		}
	}
	if (value > _maxValid) {
		value = _maxValid;
		if (_intType) {
			_lineEdit->setText(QString::number((int) value));
		}
		else {
			_lineEdit->setText(QString::number( value, 'g', _floatPrecision ));
		}
	}

	if (value == _value) return;

	_value = value;
	if (_intType) {
		emit valueChanged((int) value);
	}
	else {
		emit valueChanged(value);
	}
}

void Combo::setSlider() {
	int pos = _slider->sliderPosition();
	int min = _slider->minimum();
	int max = _slider->maximum();

	assert(min <= pos && max >= pos);

	double value = ((double) (pos - min) / (double) (max-min)) * 
		(_maxValid - _minValid) + _minValid;

	if (value == _value) return;

	_value = value;
	if (_intType) {
		emit valueChanged((int) value);
	}
	else {
		emit valueChanged(value);
	}
}

// This mini version only changes the values displayed in _lineEdit, 
// but not emit any other signals.
void Combo::setSliderMini( int pos ) {
	int min = _slider->minimum();
	int max = _slider->maximum();

	assert(min <= pos && max >= pos);

	double value = ((double) (pos - min) / (double) (max-min)) * 
		(_maxValid - _minValid) + _minValid;

	bool oldState = _lineEdit->blockSignals(true);
	if (_intType)
		_lineEdit->setText(QString::number((int) value));
	else 
	    _lineEdit->setText(QString::number( value, 'g', _floatPrecision ));
	_lineEdit->blockSignals(oldState);
}

void Combo::SetSliderLineEdit(double value) {

	// Update the class with the current min and max valid values,
	// and the new value
	//
	Update(_minValid, _maxValid, value);
}
 
void Combo::SetPrecision( int precision )
{
    if( precision > 0 )
        _floatPrecision = precision;
}

//////////////////////////////////////////////////////
//
// RangeCombo
//
//////////////////////////////////////////////////////

RangeCombo::RangeCombo(Combo* minWidget, Combo* maxWidget)
{
	_minWidget = minWidget;
	_maxWidget = maxWidget;

	connect(
		_minWidget, SIGNAL(valueChanged(double)),
		this, SLOT(setValueMin(double))
	);

	connect(
		_maxWidget, SIGNAL(valueChanged(double)),
		this, SLOT(setValueMax(double))
	);
}

void RangeCombo::Update(
	double minValid, double maxValid, double minValue, double maxValue
) {
	_minWidget->Update(minValid, maxValid, minValue);
	_maxWidget->Update(minValid, maxValid, maxValue);
}

void RangeCombo::setValueMin(double minValue) {

	double maxValue = _maxWidget->GetValue();

	// If minValue is greater than maxValue we change the maxValue to 
	// minValue
	//
	if (minValue > maxValue) {
		maxValue = minValue;
		_maxWidget->SetSliderLineEdit(maxValue);
	}

	emit valueChanged(minValue, maxValue);
}

void RangeCombo::setValueMax(double maxValue) {

	double minValue = _minWidget->GetValue();

	if (maxValue < minValue) {
		minValue = maxValue;
		_minWidget->SetSliderLineEdit(minValue);
	}

	emit valueChanged(minValue, maxValue);
}

