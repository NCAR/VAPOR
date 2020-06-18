#pragma once

#include <string>
#include <sstream>
#include <iomanip>

#include <QWidget>
#include <QMenu>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QWidgetAction>

#include "VContainer.h"

//class VSpinBoxAction;
//class VCheckBoxAction;
#include "VActions.h"
#include "VActions.h"

class VNumericFormatMenu : public QMenu {
    Q_OBJECT

public:
    explicit VNumericFormatMenu( QWidget* parent, bool sciNotation, int decimalDigits )
        : QMenu( parent ),
          _sciNotationAction( new VCheckBoxAction( "Scientific notation", sciNotation ) ),
          _decimalAction( new VSpinBoxAction( "Decimal digits", decimalDigits) ) 
    {
        connect( _sciNotationAction, &VCheckBoxAction::clicked,
            this, &VNumericFormatMenu::_sciNotationChanged );
        addAction( _sciNotationAction );

        connect( _decimalAction, &VSpinBoxAction::editingFinished,
            this, &VNumericFormatMenu::_decimalDigitsChanged );
        addAction(_decimalAction);

        /*parent->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( parent, &QWidget::customContextMenuRequested,
            this, &VNumericFormatMenu::_showMenu );*/
    }

protected:
    VCheckBoxAction* _sciNotationAction;
    VSpinBoxAction*  _decimalAction;

public:
    void SetDecimalDigits( int digits ) { _decimalAction->SetValue( digits ); }
    void SetSciNotation( bool sciNotation ) { _sciNotationAction->SetValue( sciNotation ); }
    /*void _showMenu( const QPoint& pos ) {
        std::cout << "foo" << std::endl;
        QPoint globalPos = mapToGlobal(pos);
        exec(globalPos);
    };*/

private slots:
    void _decimalDigitsChanged( int digits )     { emit DecimalDigitsChanged( digits ); }
    void _sciNotationChanged( bool sciNotation ) { emit SciNotationChanged( sciNotation ); }

signals:
    void DecimalDigitsChanged( int decimalDigits );
    void SciNotationChanged( bool sciNotation );
};

class AbstractVLineEdit : public VContainer {
    Q_OBJECT

protected:
    explicit AbstractVLineEdit( bool useNumericMenu = false) :
        VContainer(),
        _sciNotation( false ),
        _decimalDigits( 5 )
    {
        _lineEdit = new QLineEdit;//( this );
        layout()->addWidget( _lineEdit );
        connect( _lineEdit, SIGNAL( editingFinished() ),
            this, SLOT( _valueChanged() ) );

        if (useNumericMenu) {
            
            _menu = new VNumericFormatMenu( this, _sciNotation, _decimalDigits );
            //connect( this, &AbstractVLineEdit::customContextMenuRequested,
            _lineEdit->setContextMenuPolicy( Qt::CustomContextMenu );
            connect( _lineEdit, &QLineEdit::customContextMenuRequested,
                this, &AbstractVLineEdit::_showMenu );
            
            connect( _menu, &VNumericFormatMenu::SciNotationChanged,
                this, &AbstractVLineEdit::_setSciNotation );
            connect( _menu, &VNumericFormatMenu::DecimalDigitsChanged,
                this, &AbstractVLineEdit::_setDecimalDigits );
            /*_menu = new QMenu(this);
            _sciNotationAction = new VCheckBoxAction( "Scientific notation", _sciNotation );
            connect( _sciNotationAction, &VCheckBoxAction::clicked,
                this, &AbstractVLineEdit::_setSciNotation );
            _menu->addAction( _sciNotationAction );

            _decimalAction = new VSpinBoxAction( "Decimal digits", _decimalDigits);
            connect( _decimalAction, &VSpinBoxAction::editingFinished,
                this, &AbstractVLineEdit::_setDecimalDigits );
            _menu->addAction(_decimalAction);

            _lineEdit->setContextMenuPolicy( Qt::CustomContextMenu );
            connect( _lineEdit, &QLineEdit::customContextMenuRequested,
                this, &AbstractVLineEdit::_showMenu );*/
        }
    }

public slots:
    virtual void SetValue( int value ) { emit ValueChanged( value ); }
    virtual void SetValue( double value ) { emit ValueChanged( value ); }
    virtual void SetValue( const std::string& value ) { emit ValueChanged( value ); }

    // For PWidgets:
    void SetDecimalDigits( int digits ) { _decimalDigits = digits; }
    void SetSciNotation( bool sciNotation ) { _sciNotation = sciNotation; }

private slots:
    virtual void _valueChanged() = 0;
    void _setDecimalDigits( int digits ) { 
        _decimalDigits = digits;
        _valueChanged();
        emit DecimalDigitsChanged( _decimalDigits );
    }
    void _setSciNotation( bool sciNotation ) {
        _sciNotation = sciNotation;
        _valueChanged();
        emit SciNotationChanged( _sciNotation );
    }

protected:
    QLineEdit*          _lineEdit;
    //QMenu*              _menu;
    VNumericFormatMenu* _menu;
    VCheckBoxAction*    _sciNotationAction;
    VSpinBoxAction*     _decimalAction;

    bool _sciNotation;
    int  _decimalDigits;

    void _showMenu( const QPoint& pos ) {
        QPoint globalPos = mapToGlobal(pos);
        _menu->exec(globalPos);
    };

signals:
    void ValueChanged( int value );
    void ValueChanged( double value );
    void ValueChanged( const std::string& value );
    void DecimalDigitsChanged( int decimalDigits );
    void SciNotationChanged( bool sciNotation );
};

template <class T>
class VLineEdit3 : public AbstractVLineEdit {

    public:
        T GetValue() const { return _value; }
        virtual void SetValue( T value ) { 
            _value = value;
            std::string formattedNumber = _formatValue( _value );
            _lineEdit->setText( QString::fromStdString( formattedNumber ) );
        }

    protected:
        VLineEdit3( T value, bool useMenu=true ) 
        : AbstractVLineEdit(useMenu),
          _value( value )
        {
            std::string formattedNumber = _formatValue( _value );
            _lineEdit->setText( QString::fromStdString( formattedNumber ) );
        }

        virtual void _valueChanged() { 
            bool legalConversion;
            QString str = _lineEdit->text();
            double value = str.toDouble( &legalConversion );
            if (legalConversion) {
                SetValue( (T)value );
                emit AbstractVLineEdit::ValueChanged( _value ); 
            }
            else {
                SetValue( _value );
            }
        }

        std::string _formatValue( T value ) {
            std::stringstream stream;
            stream << std::fixed << std::setprecision( _decimalDigits );
            if ( _sciNotation ) {
                stream << std::scientific;
                stream << (double)_value << std::endl;
            }
            else {
                stream << _value << std::endl;
            }
            std::cout << stream.str() << std::endl;
            return stream.str();
        }
        
        T _value;

};

template <>
class VLineEdit3 <std::string> : public AbstractVLineEdit {
    protected:
        VLineEdit3( const std::string& value ) 
        : AbstractVLineEdit(),
          _value( value )
        {
            _lineEdit->setText( QString::fromStdString( value ) );
        }
        
        virtual void SetValue( const std::string& value ) { 
            _value = value; 
            _lineEdit->setText( QString::fromStdString( value ) );
        }

        std::string GetValue() const { return _value; }

    //protected:
        std::string _value;
        virtual void _valueChanged() { 
            std::string value = _lineEdit->text().toStdString();
            if ( value != _value ) {
                _value = value;
                emit AbstractVLineEdit::ValueChanged( _value ); 
            }
        }
};

class VIntLineEdit3 : public VLineEdit3<int> {
    public:
        VIntLineEdit3( int value, bool useMenu=true ) 
        : VLineEdit3<int>(value, useMenu) {}
};

class VDoubleLineEdit3 : public VLineEdit3<double> {
    public:
        VDoubleLineEdit3( double value, bool useMenu=true ) 
        : VLineEdit3<double>(value, useMenu) {}
};

class VStringLineEdit3 : public VLineEdit3<std::string> {
    public:
        VStringLineEdit3( const std::string& value ) 
        : VLineEdit3<std::string>(value) {}
};
