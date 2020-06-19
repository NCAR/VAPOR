#pragma once

#include "VNumericFormatMenu.h"

class VIntLineEditAction;

class VIntRangeMenu : public VNumericFormatMenu {
    Q_OBJECT

public:
    explicit VIntRangeMenu( 
        QWidget* parent, 
        bool sciNotation, int decimalDigits,
        int min, int max 
    );

protected:
    VIntLineEditAction* _minRangeAction;
    VIntLineEditAction* _maxRangeAction;

public:
    void SetMinRange( int min );
    void SetMaxRange( int max );

private slots:
    void _minChanged( int min );
    void _maxChanged( int max );

signals:
    void MinChanged( int min );
    void MaxChanged( int max );
};
