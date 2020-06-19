#pragma once

#include <string>
#include "VContainer.h"

class QMenu;
class VSlider;
class VNumericLineEdit;
class VIntLineEditAction;
class VIntLineEdit;
class VDoubleLineEditAction;
class VCheckBoxAction;
class VSpinBoxAction;

class AbstractVSliderEdit : public VContainer {
    Q_OBJECT

protected:
    explicit AbstractVSliderEdit() :
        VContainer(),
        _sciNotation( false ),
        _decimalDigits( 5 )
    {}

    void Initialize( const std::string& title ) {
        //_lineEdit = new QLineEdit;//( this );
        layout()->addWidget( _lineEdit );
        connect( _lineEdit, SIGNAL( editingFinished() ),
            this, SLOT( _valueChanged() ) );

        if (useNumericMenu) {
            _menu = new QMenu(this);
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
                this, &AbstractVLineEdit::_showMenu );
        }
    }

public slots:
    virtual void SetValue( int value ) { emit ValueChanged( value ); }
    virtual void SetValue( double value ) { emit ValueChanged( value ); }

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
    AbstractVLineEdit* _lineEdit;
    VSlider*           _slider;

    bool               _sciNotation;
    VCheckBoxAction*   _sciNotationAction;

    int                _decimalDigits;
    VSpinBoxAction*    _decimalAction;

    void _showMenu( const QPoint& pos ) {
        QPoint globalPos = mapToGlobal(pos);
        _menu->exec(globalPos);
    };

signals:
    void ValueChanged( int value );
    void ValueChanged( double value );
    void DecimalDigitsChanged( int decimalDigits );
    void SciNotationChanged( bool sciNotation );
};

class VSliderEdit2 : public VContainer {
    Q_OBJECT

public:
    virtual void SetSciNotation( bool sci ) = 0;
    virtual void SetNumDigits( int digits ) = 0;

    virtual bool   GetSciNotation() const = 0;
    virtual int    GetNumDigits() const = 0;

protected:
    VSliderEdit2();

    VNumericLineEdit* _lineEdit;
    VSlider*          _slider;

    //QMenu*                 _menu;
    VIntRangeMenu*         _menu;
    VSpinBoxAction*        _decimalAction;
    VCheckBoxAction*       _scientificAction;

public slots:
    void ShowContextMenu( const QPoint& );

protected slots:
    void _showMenu( const QPoint& pos ) {
        QPoint globalPos = mapToGlobal(pos);
        _menu->exec(globalPos);
    };

signals:
    void FormatChanged();
};


class VIntSliderEdit : public VSliderEdit2 {
    Q_OBJECT

public:
    VIntSliderEdit( int min=0, int max=10, int value=3 );

    void SetValue( int value );
    void SetMinimum( int min );
    void SetMaximum( int max );

    int GetValue() const;
    int GetMinimum() const;
    int GetMaximum() const;
    
    virtual int  GetNumDigits() const;
    virtual void SetNumDigits( int numDigits );

    virtual bool GetSciNotation() const;
    virtual void SetSciNotation( bool sciNotation );

protected:
    virtual void _makeSliderEdit();
    virtual void _makeContextMenu();
    void _sliderChanged( int value );
    void _sliderChangedIntermediate( int value );

    //int _min;
    //int _max;
    int _value;    

    VIntLineEdit* _lineEdit;
    VIntLineEditAction* _minRangeAction;
    VIntLineEditAction* _maxRangeAction;
    

signals:
    void ValueChanged( int value );
    void ValueChangedIntermediate( int value );
    void MinimumChanged( int min );
    void MaximumChanged( int max );
};
