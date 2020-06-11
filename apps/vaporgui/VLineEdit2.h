#pragma once

#include <string>

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QWidgetAction>

#include "VContainer.h"

//! \class VLineEdit
//!
//! Wraps a QLineEdit with vaporgui's standard setter/getter functions.
//! Handles string and double types, as well as precision and display of
//! double values.

class VLESignals : public QObject {
    Q_OBJECT

protected:
    VLESignals();
    QLineEdit*  _lineEdit;

slots:
    void emitChange();

signals:
    void ValueChanged( int );
    virtual void ValueChanged( double );
    virtual void ValueChanged( std::string );
};

template <class T>
//class VLineEdit2 : public VLESignals, public VContainer {
class VLineEdit2 : public VLESignals, public VContainer {

public:
    VLineEdit2( const T& value );
    T    GetValue() const;
    void SetValue( T value );
    //void emitLineEditChanged();

protected:
    //VLineEdit2( const T& value );
    T           _value;
};



class VIntLineEdit : public VLineEdit2<int> {

public:
    VIntLineEdit( int value );
    void SetValue( int value );
    //int          GetValue() { return _value; }
    //virtual void UseContextMenu() { std::cout << "VIntLineEdit::UseContextMenu()" << std::endl }
    //virtual void emitLineEditChanged() { emit ValueChanged( int ); }
};

class VDoubleLineEdit : public VLineEdit2<double> {

public:
    VDoubleLineEdit( double value );
    void SetValue( double value );
};

class VStringLineEdit : public VLineEdit2<std::string> {

public:
    VStringLineEdit( std::string value );
    void SetValue( std::string value );
};
