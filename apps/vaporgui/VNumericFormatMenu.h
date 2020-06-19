#pragma once

#include <QMenu>

class VCheckBoxAction;
class VSpinBoxAction;

class VNumericFormatMenu : public QMenu {
    Q_OBJECT

public:
    explicit VNumericFormatMenu( QWidget* parent, bool sciNotation, int decimalDigits );
    void SetDecimalDigits( int digits );
    void SetSciNotation( bool sciNotation );

protected:
    VCheckBoxAction* _sciNotationAction;
    VSpinBoxAction*  _decimalAction;

private slots:
    void _decimalDigitsChanged( int digits );
    void _sciNotationChanged( bool sciNotation );

signals:
    void DecimalDigitsChanged( int decimalDigits );
    void SciNotationChanged( bool sciNotation );
};
