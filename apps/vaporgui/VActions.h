#pragma once

#include <string>

#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QWidgetAction>
#include <QHBoxLayout>

#include "VIntSpinBox.h"
#include "VCheckBox.h"
#include "VIntLineEdit.h"
#include "VDoubleLineEdit.h"
#include "VStringLineEdit.h"
//#include "VLineEdit2.h"

class VSpinBoxAction : public QWidgetAction {
    Q_OBJECT

public:
    VSpinBoxAction (const std::string& title, int value) : 
      QWidgetAction (NULL) {

        _spinBox = new VIntSpinBox(1, 10);
        _spinBox->SetValue( value );

        VLineItem* vli = new VLineItem( title, _spinBox );
        vli->setContentsMargins( 5, 0, 5, 0 );

        connect( _spinBox, SIGNAL( ValueChanged( int ) ),
            this, SLOT( spinBoxChanged( int ) ) );

        setDefaultWidget(vli);
    }

    int GetValue() const {
        return _spinBox->GetValue();
    }

    void SetValue( int value ) {
        _spinBox->SetValue( value );
    }

private:
    VIntSpinBox* _spinBox;

private slots:
    void spinBoxChanged( int value ) {
        emit editingFinished( value );
    }

signals:
    void editingFinished( int );
};

class VCheckBoxAction : public QWidgetAction {
    Q_OBJECT

public:
    VCheckBoxAction (const std::string& title, bool value) : 
      QWidgetAction (NULL) {

        _checkBox = new VCheckBox( value );
        VLineItem* vli = new VLineItem( title, _checkBox );
        vli->setContentsMargins( 5, 0, 5, 0 );

        connect( _checkBox, &VCheckBox::ValueChanged,
            this, &VCheckBoxAction::checkBoxChanged );

        setDefaultWidget(vli);
    }

    bool GetValue() const {
        return _checkBox->GetValue();
    }
    
    void SetValue( bool value ) {
        _checkBox->SetValue( value );
    }

private:
    VCheckBox* _checkBox;

private slots:
    void checkBoxChanged( bool value ) {
        emit clicked( value );
    }

signals:
    void clicked( bool );
};


class VStringLineEditAction : public QWidgetAction {
    Q_OBJECT

public:
    VStringLineEditAction( const std::string& title, std::string value ) :
      QWidgetAction(nullptr) {
        _lineEdit = new VStringLineEdit( value ); //std::to_string( value ) );
        VLineItem* vli = new VLineItem( title, _lineEdit );
        vli->setContentsMargins( 5, 0, 5, 0 );

        connect( _lineEdit, SIGNAL( ValueChanged( int ) ),
            this, SLOT( _lineEditChanged( int ) ) );
    
        setDefaultWidget( vli );
    }

    void SetValue( const std::string& value ) {
        _lineEdit->SetValue( value );
    }

private:
    VStringLineEdit* _lineEdit;

private slots:
    void _lineEditChanged( int value ) {
        emit ValueChanged( value );
    }

signals:
    void ValueChanged( int );
};

class VIntLineEditAction : public QWidgetAction {
    Q_OBJECT

public:
    VIntLineEditAction( const std::string& title, int value ) :
      QWidgetAction(nullptr) {
        _lineEdit = new VIntLineEdit( value );
        VLineItem* vli = new VLineItem( title, _lineEdit );
        vli->setContentsMargins( 5, 0, 5, 0 );

        connect( _lineEdit, SIGNAL( ValueChanged( int ) ),
            this, SLOT( _lineEditChanged( int ) ) );
    
        setDefaultWidget( vli );
    }

    void SetValue( int value ) {
        _lineEdit->SetValue( value );
    }

private:
    VIntLineEdit* _lineEdit;

private slots:
    void _lineEditChanged( int value ) {
        emit ValueChanged( value );
    }

signals:
    void ValueChanged( int );

};

class VDoubleLineEditAction : public QWidgetAction {
    Q_OBJECT

public:
    VDoubleLineEditAction( const std::string& title, double value ) :
      QWidgetAction(nullptr) {
        _lineEdit = new VDoubleLineEdit( value );
        VLineItem* vli = new VLineItem( title, _lineEdit );
        vli->setContentsMargins( 5, 0, 5, 0 );

        connect( _lineEdit, SIGNAL( ValueChanged( double ) ),
            this, SLOT( _lineEditChanged( double ) ) );
    
        setDefaultWidget( vli );
    }
    
    void SetValue( double value ) {
        _lineEdit->SetValue( value );
    }

private:
    VDoubleLineEdit* _lineEdit;

private slots:
    void _lineEditChanged( double value ) {
        emit ValueChanged( value );
    }

signals:
    void ValueChanged( double );
};
