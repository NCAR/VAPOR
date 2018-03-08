#include <QtGui>
#include <iostream>
#include <cassert>

using namespace std;

#include "RangeCombos.h"


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

